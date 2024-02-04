# Willow

<img src="willow_logo.png" width="300" alt="Willow's logo">

The strongest mailbox chess engine in the world (mailbox being a board representation and an alternative to bitboards). Willow started out as a passion project, written entirely from scratch, to 

# Features
Search: Willow is a relatively standard alpha-beta engine as far as search goes. It boasts all the major search algorithms such as Quiescence Search, Aspiration Windows, and the like; and also features many "lesser" search techniques such as material scaling, several different types of history, etc. Although the general algorithms are the same, the exact details of their implementation (depth reduction, conditions in when to use them, etc), are unique to each engine, and Willow is no exception - hundreds of hours were spent into finding optimal parameters and subconditions to squeeze every last drop of Elo out of these features.

Eval: Willow is a NNUE (formerly HCE) engine trained on 2b positions of self-play data. It used to feature a unique method of introducing noise into the training dataset by occasionally making a combination of random moves and calculated mistakes, however this methodology has since been abandoned in the quest for greater strength. The hunt for an aggressive yet strong engine will be resumed far more thoroughly in my next engine, the [Patricia Project](https://github.com/Adam-Kulju/Patricia).

# Contributing
If you want to write a patch for Willow, there's lots of things to work on - besides strength gains, I'll accept stuff as simple as making this Readme look nicer or cleaning up some code. If you are interested in an elo gainer, just fork the repo and edit it however you want, then contact me to get a test set up. 

If you want to donate hardware that will go towards testing patches for the engine, you can give compute by getting a client file for [Willow's OpenBench Instance](https://github.com/crippa1337/OpenBench) and creating a profile [here](https://chess.swehosting.se/).

# Acknowledgements
Thank you to all my friends in the OpenBench instance: @cosmobobak, @uwuplant, @crippa1337, @archishou, @Ciekce, @JouanDeag, @GediminasMasaitis, @Alex2262, @lucametehau, @spamdrew128, @lucametehau, @PGG106, @JacquesRW and @dede1751. In addition to providing compute to the instance, allowing Willow to be tested quickly and easily, they have also provided me with so many ideas, as well as inspired me and also simply made my journey through chess programming a fun as well as fulfilling venture.
