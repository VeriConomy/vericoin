#!/usr/bin/env bash

export LC_ALL=C
TOPDIR=${TOPDIR:-$(git rev-parse --show-toplevel)}
BUILDDIR=${BUILDDIR:-$TOPDIR}

BINDIR=${BINDIR:-$BUILDDIR/src}
MANDIR=${MANDIR:-$TOPDIR/doc/man}

VERICOIND=${VERICOIND:-$BINDIR/vericoind}
VERICOINCLI=${VERICOINCLI:-$BINDIR/vericoin-cli}
VERICOINTX=${VERICOINTX:-$BINDIR/vericoin-tx}
WALLET_TOOL=${WALLET_TOOL:-$BINDIR/vericoin-wallet}
VERICOINQT=${VERICOINQT:-$BINDIR/qt/vericoin-qt}

[ ! -x $VERICOIND ] && echo "$VERICOIND not found or not executable." && exit 1

# The autodetected version git tag can screw up manpage output a little bit
read -r -a BTCVER <<< "$($VERICOINCLI --version | head -n1 | awk -F'[ -]' '{ print $5, $6 }')"

# Create a footer file with copyright content.
# This gets autodetected fine for vericoind if --version-string is not set,
# but has different outcomes for vericoin-qt and vericoin-cli.
echo "[COPYRIGHT]" > footer.h2m
$VERICOIND --version | sed -n '1!p' >> footer.h2m

for cmd in $VERICOIND $VERICOINCLI $VERICOINTX $WALLET_TOOL $VERICOINQT; do
  cmdname="${cmd##*/}"
  help2man -N --version-string=${BTCVER[0]} --include=footer.h2m -o ${MANDIR}/${cmdname}.1 ${cmd}

  if [ -n "${BTCVER[1]}" ]; then
    sed -i "s/\\\-${BTCVER[1]}//g" ${MANDIR}/${cmdname}.1
  fi
done

rm -f footer.h2m
