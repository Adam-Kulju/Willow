#ifndef __nnue__
#define __nnue__
#include "constants.h"
#include "globals.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <span>
#include <vector>
#ifdef _MSC_VER
#define W_MSVC
#pragma push_macro("_MSC_VER")
#undef _MSC_VER
#endif

#define INCBIN_PREFIX g_
#include "incbin.h"

#ifdef W_MSVC
#pragma pop_macro("_MSC_VER")
#undef W_MSVC
#endif

class Position;

constexpr size_t INPUT_SIZE = 768 * 4;
constexpr size_t LAYER1_SIZE = 768;

constexpr int SCRELU_MIN = 0;
constexpr int SCRELU_MAX = 255;

constexpr int SCALE = 400;

constexpr int QA = 255;
constexpr int QB = 64;

constexpr int QAB = QA * QB;

constexpr int STANDARD_TO_MAILBOX[64] = {
    0x0,  0x1,  0x2,  0x3,  0x4,  0x5,  0x6,  0x7,  0x10, 0x11, 0x12,
    0x13, 0x14, 0x15, 0x16, 0x17, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x40,
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x50, 0x51, 0x52, 0x53,
    0x54, 0x55, 0x56, 0x57, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77};

constexpr int MAILBOX_TO_STANDARD[0x80] = {
    56, 57, 58, 59, 60, 61, 62, 63, 99, 99, 99, 99, 99, 99, 99, 99, 48, 49, 50,
    51, 52, 53, 54, 55, 99, 99, 99, 99, 99, 99, 99, 99, 40, 41, 42, 43, 44, 45,
    46, 47, 99, 99, 99, 99, 99, 99, 99, 99, 32, 33, 34, 35, 36, 37, 38, 39, 99,
    99, 99, 99, 99, 99, 99, 99, 24, 25, 26, 27, 28, 29, 30, 31, 99, 99, 99, 99,
    99, 99, 99, 99, 16, 17, 18, 19, 20, 21, 22, 23, 99, 99, 99, 99, 99, 99, 99,
    99, 8,  9,  10, 11, 12, 13, 14, 15, 99, 99, 99, 99, 99, 99, 99, 99, 0,  1,
    2,  3,  4,  5,  6,  7,  99, 99, 99, 99, 99, 99, 99, 99,
};

struct alignas(64) NNUE_Params {
  std::array<int16_t, INPUT_SIZE * LAYER1_SIZE> feature_weights;
  std::array<int16_t, LAYER1_SIZE> feature_bias;
  std::array<int16_t, LAYER1_SIZE * 2> output_weights;
  int16_t output_bias;
};

extern const NNUE_Params &g_nnue;

template <size_t HiddenSize> struct alignas(64) Accumulator {
  std::array<int16_t, HiddenSize> white;
  std::array<int16_t, HiddenSize> black;

  inline void init(std::span<const int16_t, HiddenSize> bias) {
    std::memcpy(white.data(), bias.data(), bias.size_bytes());
    std::memcpy(black.data(), bias.data(), bias.size_bytes());
  }
  inline void init_color(std::span<const int16_t, HiddenSize> bias, int color) {
    if (color){
      std::memcpy(black.data(), bias.data(), bias.size_bytes());
    }
    else{
      std::memcpy(white.data(), bias.data(), bias.size_bytes());
    }
  }
};

constexpr int32_t screlu(int16_t x) {
  const auto clipped =
      std::clamp(static_cast<int32_t>(x), SCRELU_MIN, SCRELU_MAX);
  return clipped * clipped;
}

class NNUE_State {
public:
  explicit NNUE_State() { m_accumulator_stack.reserve(100); }

  ~NNUE_State() = default;

  void push();

  void pop();

  template <bool Activate> inline void update_feature(int piece, int square, int wkbucket, int bkbucket) {
    const auto [white_idx, black_idx] = feature_indices(piece, square);

    if constexpr (Activate) {
      add_to_all(m_curr->white, g_nnue.feature_weights,
                 (white_idx + 768*wkbucket) * LAYER1_SIZE);
      add_to_all(m_curr->black, g_nnue.feature_weights,
                 (black_idx + 768*bkbucket) * LAYER1_SIZE);
    } else {
      subtract_from_all(m_curr->white, g_nnue.feature_weights,
                        (white_idx + 768*wkbucket) * LAYER1_SIZE);
      subtract_from_all(m_curr->black, g_nnue.feature_weights,
                        (black_idx + 768*bkbucket) * LAYER1_SIZE);
    }
  }

  template <bool Activate> inline void update_feature_color(int piece, int square, int color, int bucket) {
    const auto [white_idx, black_idx] = feature_indices(piece, square);

    int indx = color ? black_idx : white_idx;

    if constexpr (Activate) {
      add_to_all(color ? m_curr->black : m_curr->white, g_nnue.feature_weights,
                 (indx + 768*bucket) * LAYER1_SIZE);
    } else {
      subtract_from_all(color ? m_curr->black : m_curr->white, g_nnue.feature_weights,
                 (indx + 768*bucket) * LAYER1_SIZE);
    }
  }

  [[nodiscard]] int evaluate(int color) const;

  std::vector<Accumulator<LAYER1_SIZE>> m_accumulator_stack{};
  Accumulator<LAYER1_SIZE> *m_curr{};

  template <size_t size, size_t weights>
  inline void add_to_all(std::array<int16_t, size> &input,
                         const std::array<int16_t, weights> &delta,
                         size_t offset) {
    for (size_t i = 0; i < size; ++i) {
      input[i] += delta[offset + i];
    }
  }

  template <size_t size, size_t weights>
  inline void subtract_from_all(std::array<int16_t, size> &input,
                                const std::array<int16_t, weights> &delta,
                                size_t offset) {
    for (size_t i = 0; i < size; ++i) {
      input[i] -= delta[offset + i];
    }
  }

  static std::pair<size_t, size_t> feature_indices(int piece, int sq);

  static int32_t
  screlu_flatten(const std::array<int16_t, LAYER1_SIZE> &us,
                const std::array<int16_t, LAYER1_SIZE> &them,
                const std::array<int16_t, LAYER1_SIZE * 2> &weights);

  void reset_nnue(struct board_info *board);
  void reset_nnue_color(struct board_info *board, int color);
};

INCBIN(nnue, "src/ida.nnue");
const NNUE_Params &g_nnue = *reinterpret_cast<const NNUE_Params *>(g_nnueData);

void NNUE_State::push() {
  m_accumulator_stack.push_back(*m_curr);
  m_curr = &m_accumulator_stack.back();
  // printf("makemove - %i\n", nnue_state.evaluate(WHITE));
}

void NNUE_State::pop() {
  m_accumulator_stack.pop_back();
  m_curr = &m_accumulator_stack.back();
  // printf("unmakemove - %i\n", nnue_state.evaluate(WHITE));
  // exit(0);
}

int NNUE_State::evaluate(int color) const {
  const auto output =
      color == WHITE
          ? screlu_flatten(m_curr->white, m_curr->black, g_nnue.output_weights)
          : screlu_flatten(m_curr->black, m_curr->white, g_nnue.output_weights);
  return (output + g_nnue.output_bias) * SCALE / QAB;
}

std::pair<size_t, size_t> NNUE_State::feature_indices(int piece, int sq) {
  constexpr size_t color_stride = 64 * 6;
  constexpr size_t piece_stride = 64;

  const auto base = static_cast<int>(piece / 2 - 1);
  const size_t color = piece & 1;

  // std::cout << piece << " " << sq << " " << base << " " << color <<
  // std::endl;
  const auto whiteIdx =
      color * color_stride + base * piece_stride + static_cast<size_t>(sq ^ 56);
  const auto blackIdx = (color ^ 1) * color_stride + base * piece_stride +
                        (static_cast<size_t>(sq));

  // std::cout << whiteIdx << " " << blackIdx << std::endl;
  return {whiteIdx, blackIdx};
}

int32_t
NNUE_State::screlu_flatten(const std::array<int16_t, LAYER1_SIZE> &us,
                          const std::array<int16_t, LAYER1_SIZE> &them,
                          const std::array<int16_t, LAYER1_SIZE * 2> &weights) {
  int32_t sum = 0;

  for (size_t i = 0; i < LAYER1_SIZE; ++i) {
    sum += screlu(us[i]) * weights[i];
    sum += screlu(them[i]) * weights[LAYER1_SIZE + i];
  }

  return sum / QA;
}

void NNUE_State::reset_nnue(struct board_info *board) {
  m_accumulator_stack.clear();
  m_curr = &m_accumulator_stack.emplace_back();

  m_curr->init(g_nnue.feature_bias);

  for (int square : STANDARD_TO_MAILBOX) {
    if (board->board[square]) {
      // printf("%i\n", MAILBOX_TO_STANDARD[square]);
      update_feature<true>(board->board[square], MAILBOX_TO_STANDARD[square], buckets[WHITE][board->kingpos[WHITE]], buckets[BLACK][board->kingpos[BLACK]]);
    }
  }
}

void NNUE_State::reset_nnue_color(struct board_info *board, int color) {
  m_curr->init_color(g_nnue.feature_bias, color);

  for (int square : STANDARD_TO_MAILBOX) {
    if (board->board[square]) {
      // printf("%i\n", MAILBOX_TO_STANDARD[square]);
      update_feature_color<true>(board->board[square], MAILBOX_TO_STANDARD[square], color, buckets[color][board->kingpos[color]]);
    }
  }
}

#endif