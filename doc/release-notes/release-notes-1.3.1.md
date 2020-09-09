Verium Vault version 1.3.1 is now available from:

  https://vericoin.info

This is a new major version release, bringing both new features and
bug fixes.

Please report bugs using the issue tracker at github:

  https://github.com/VeriumReserve/Verium

Upgrading and downgrading
=========================

How to Upgrade
--------------

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions), uninstall all
earlier versions of Verium, then run the installer.

We recommand before any upgrade that you backup your wallet.

If you are upgrading from version 1.2 or earlier, the first time you run
1.3.0 your blockchain files will be re-indexed, which will take anywhere from
5 minutes to several hours, depending on the speed of your machine.

Downgrading warnings
--------------------

The 'chainstate' for this release is not always compatible with previous
releases, so if you run 1.3.x and then decide to switch back to a
1.2.x release you might get a blockchain validation error when starting the
old release (due to 'pruned outputs' being omitted from the index of
unspent transaction outputs).

Running the old release with the -reindex option will rebuild the chainstate
data structures and correct the problem.

Also, the first time you run a 1.2.x release on a 1.3.x wallet it will rescan
the blockchain for missing spent coins, which will take a long time (tens
of minutes on a typical machine).

Notable changes
===============

VIP: Fee change
-----------------------

Fees will change from a min of 0.2 VRM/kb to 0.001 VRM/kb at block 520000

1.3.1 Change log
=======================

RPC:

- Fix testmempoolaccept with absurd fee
- Increase absurd fee to 10 VRM

Block and transaction handling:

- Cleanup

Mining:

- 'getblocktemplate' does not require segwit key

Protocol and network:

- Protocol 80003
- New Fee system

Validation:

- Validation that transaction respect minimum fee

GUI:

- Add Revision version on splash screen (the last number on v1.3.1<-)

Build system:

- AppImage for unix

Credits
--------

Thanks to everyone who contributed to this release:
- Bitcoin Core Team
- Armaos
- hd4r
- Douglas Pike (effectsToCause)
- Matthieu Derasse (mderasse)

And thanks to the community to keep following us and all their work