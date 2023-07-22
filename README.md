# Willow
The strongest mailbox chess engine in the world.

# Features
Search: Willow is a relatively standard alpha-beta engine as far as search goes. It has all the major algorithms that major engines use; from the most important ones like Alpha-Beta Pruning with Aspiration Windows, Reverse Futility Pruning, Transposition Tables, Quiescence Search, etc. to more obscure ones such as Singular Extensions, Countermove History and Delta Pruning in Qsearch. Although the general algorithms are the same, the exact details of their implementation (depth reduction, conditions in when to use them, etc), are unique to each engine, and Willow is no exception; I have thrown stuff at the LMR code hundreds of times for example in an attempt to squeeze some more ELO out of it.

Eval: Willow is a NNUE (formerly HCE) engine trained with a unique methodology to promote aggressive play. While generating data for self-play games, Willow will intentionally make mistakes sometimes, ranging from moves that are just slightly worse than the best move, all the way down to blunders and random moves. I got the idea from a Leela Chess Zero project developer mentioning that they make their data, "play like a drunk grandmaster", but the vast majority of engines nonetheless still simply play the best move all the time while generating data.

Introducing error into the training dataset has two benefits:
1. It promotes an aggressive style of play by making wins in data generation more likely in positions where there is only one good move to defend (which happens in most attacking games), which in turn leads to the NNUE preferring aggressive/attacking positions over more quiet ones.
2. The errors and random moves force the engine to search a wider variety of positions, reducing overhead and hypothetically making the engine stronger.

My goal in the future is to further refine the manner in which I go about introducing uncertainty into the training dataset, and to see other engines follow in Lc0's footprints.

#Acknowledgements

Thank you to all my friends in the OpenBench instance: @cosmobobak, @uwuplant, @crippa1337, @archishou, @Ciekce, @JouanDeag, @GediminasMasaitis, @Alex2262, @lucametehau, @spamdrew128, @lucametehau, @PGG106, @JacquesRW and @dede1751. In addition to providing compute to the instance, allowing Willow to be tested quickly and easily, they have also provided me with so many ideas, as well as inspired me and also simply made my journey through chess programming a fun as well as fulfilling venture.

Special thanks for @Alex2262 for helping me generate data for the NNUE that Willow is trained on.


# Logo

<img src="LOGO.png" width="300" alt="Willow's logo">
