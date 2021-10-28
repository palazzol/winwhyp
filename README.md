# WinWHYP

Port of the WHYP Forth interface for DOS, to Windows (Visual Studio 2019)

My former teacher, [Professor Richard E. Haskell](http://www.richardhaskell.com/), developed a Forth
environment for teaching embedded systems and digital design in the 1990s at [Oakland University](https://oakland.edu/).
It is called WHYP for "Words to Help You Program."

It is implemented on the Motorola 68HC11 and 68HC12 family of chips, and the interface is a DOS program.

It is fully described in [his book](https://www.amazon.com/Design-Embedded-Systems-68HC12-Microcontrollers/dp/0130832081)
which you can probably find on eBay

The original code and a couple papers are still available [here](http://www.cse.secs.oakland.edu/haskell/hc12_book.htm)

The DOS software can run in DOSBOX. I have ported the DOS software to Windows.  
I hope to make a from new multi-platform Python version, so this is really just a stopgap and an easy way to run the software without an emulator.

## Notes:

This port is a WIP and not tested very well.

## Known Bugs:

The COM port is hardcoded!!  I will fix this soon but for now you need to edit [this line](https://github.com/palazzol/winwhyp/blob/main/src/UART.CPP#L76) and recompile.

## License:

All this software is (c) Richard E. Haskell, I am just trying to keep it going.
If you like this, take a look at the books and software that he is selling on [his website](http://www.richardhaskell.com/).
