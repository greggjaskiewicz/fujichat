#!/usr/bin/perl -w

use bytes;

$c = 0;

while(<>) {
	next unless @bytes = (/0x([0-9a-fA-F]{2})/g);
	for(@bytes) {
		if(!($c % 8)) {
			print "        .byte ";
		}

		printf "\$%02X", reverse_bits(hex $_);

		if(($c % 8 == 7) || ($c == $#bytes)) {
			print "\n";
			$c = 0;
		} else {
			print ",";
			$c++;
		}
	}
}

sub reverse_bits {
	my $bitstr = reverse sprintf("%08b", $_[0]);
	return eval "0b$bitstr";
}
