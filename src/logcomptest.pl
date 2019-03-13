#!/usr/bin/perl -w

# sample session for IRC log compression scheme

$lastnick = "";
$textlen = 0;
while(<DATA>) {
	$lines++;
	$bigtot += length($_);
	chomp;

	my ($nick, $text) = split " ", $_, 2;
	for($nick, $text) {
		s/(.)$/chr(ord($1)|0x80)/e;
	}

	if(($nick ne $lastnick) || ($textlen + length($text) > 256)) {
		$output .= $nick;
		$output .= $text;
		$lastnick = $nick;
		$textlen = length($text);
	} else {
		$output .= chr(1) . $text;
		$textlen += length($text);
	}
}

print "lines: $lines\n";
print "old: $bigtot\n";
print "new: " . length($output) . "\n";
print "saved: " . ($bigtot - length($output)) . "\n";
print "compression: " . (1.0 - (length($output) / $bigtot)) . "\n";

print STDERR $output;

__DATA__
Slor WIDE?
Slor dunno
Redbeard I've seen a 29w7 though.
SteveS who you calling WIDE?
DogWombl i'm not wide, i'm just badly out of proportion
Redbeard Was labeled "video" on the back of a Intergraph machine, back when they had their own unix running on a custom cpu.
Slor where'd urch go, anyway?
Slor ask a question and disappear
SteveS he said he had real work
SteveS be back late, late
BeetlePC hey SteveSI have two 10base2 NICs but I don't need transceivers and I definitely don't want any of those fat ass transceiver cables.
SteveS beetle, how is the 1080 coming?
SteveS bah.  I'm going through BSG withdrawal!
Redbeard Transcievers would allow you to put a 10base5 nic onto the same coax network. Or sometimes rackmount switches have a (is it db15?) connector on them.
Slor someone tell me this too - what is a "Modem Eliminator"?  Is that just fancy for null modem?
SteveS group shrug
BeetlePC i didn't work on it lately
SteveS grrr.  I still can't figure why my MyIDE movie audio player is not playing nice.
SteveS I will order parts for SDrive this week.  I will. I will. I will.
Redbeard q
SteveS been saying that for a while now
BeetlePC good, good, good
BeetlePC :)
Slor doesn't buy it
SteveS $7 seems too much for a SD connector though.   I might try the floppy cable connector hack on the first try.
BeetlePC you can also cannibalize a dead/cheap SD card reader
SteveS don't have one
ChrisTOS nn
BeetlePC night
SteveS nite
SteveS christos
ChrisTOS q
Slor didn't even see him talk today
SteveS I need to get a working one though
atarixle q
SteveS I didn't know atarixle was here
SteveS Would ask him if there is an English version of the new Boss X.
BeetlePC he told you
SteveS people need better names, btw.   XL Boss, Boss XL, and Boss X.
BeetlePC 10.4 will be available in german and english
SteveS oh!
SteveS I missed the answer.
SteveS I thought he ignored me
BeetlePC i remembered it :)
SteveS my mistake.  sorry.
BeetlePC np
BeetlePC gets himself a beer
BeetlePC cheers
SteveS has no beer in the house and is not going out in 19 degree weather to get some.
SteveS cheers
SteveS holds up his empty tea mug
BeetlePC :)
BeetlePC *pling*
BeetlePC i scored an IBM thinkpad T42 on ebay today
BeetlePC its for my dad
SteveS Zum Wohl to dad
BeetlePC 1.6 GHz Pentium M, 1 Gig of RAM and a new 160GB HD, plus a new IBM battery (used laptop batterys are most often dead)
BeetlePC should do for his mobile computing needs
SteveS :)
BeetlePC He is some 'car accident expert' i don't know how that job is called in english. He views damaged cars and estimates the cost of repairs. He also reconstructs what happend during the crash.
BeetlePC How the crash happend
BeetlePC happened
SteveS Adjuster?
SteveS Insurance Adjuster, is what we call them.
BeetlePC hmm
BeetlePC he does not work for an insurance
SteveS oh
BeetlePC one-man-company
BeetlePC he is the boss of himself :)
BeetlePC and also the best boy
SteveS I don't think we have a single equivalent for that job.
BeetlePC Kfz-Sachverständiger
BeetlePC lets see what google says
BeetlePC car expert :-/
SteveS oops when I cut and pasted  into babel fish I got my Atari font
SteveS Kfz-Sachverständiger
SteveS looks Ok here
SteveS Atari Extrasmooth
BeetlePC well, he looks at the damaged car, writes down every harmed component and calculates the time to repair it.
SteveS in babel fish
BeetlePC and for this job, a laptop would be nice
SteveS estimates are performed by a representative of your insurance company here.
BeetlePC thats common here, too
SteveS also called Claims Adjuster
BeetlePC but the job has not to be done by someone from the insurance
BeetlePC you are allowed to choose here
SteveS some insurance companines allow you to call to a service shop or even several service shops.
BeetlePC and a 'independend claims adjuster' might get a larger payment form the insurance of your opponent than his insurances representant
BeetlePC from -from
BeetlePC because he doesn't get payed for keeping costs small for the insurance
SteveS Some insurance companies won't even pay for original parts
BeetlePC because of that, if your car gets damaged by someone, you are allowed to choose who will examine you car
rrCurtisP j
SteveS My friend bought a Mini Cooper
SteveS a new one
BeetlePC wow, nice
SteveS I think he is a having a mid-life crisis.   Normally a very conservative person.
SteveS I always wanted a Porshe 356
BeetlePC well, if a car he likes helps - better than leaviing wife and kids as a neighour here does
SteveS ouch
BeetlePC fuse blown - i have no other explanation
SteveS 20 yr-old GF maybe
BeetlePC no - or he does hide it well
BeetlePC :)
charliec j
charliec good evening all
BeetlePC hey charliec
charliec Hiya Beetle
Harlock q
SteveS hey charliec
charliec Hi Steve
SteveS charliie, did I remember to send you MyDOS 3.19?
charliec Nope
SteveS It has the RS232 driver for ATR8000
charliec didnt know if you caught that..
SteveS sorry
charliec dont be
SteveS dcc in just a second
charliec hold.. im on fuji
SteveS oh
SteveS that won't work
SteveS not yet
charliec not yet huh
cc_PC j
cc_PC okee
cc_PC ready when you are
Harlock j
cc_PC Thanks!
SteveS welcome
SteveS you will be asked to build a version of DOS.  It does it in memory the you must select 'H', Save DOS files.
cc_PC Ok
charliec ever run CP/M on the ATR Steve?
Moloch j
charliec well heading to a surprise b-day party, cant be late for those!
charliec nute
charliec heh.. nite
charliec q
cc_PC q
BeetlePC good night
BeetlePC q
fres j
fres hey, all.
fres and by all I mean, uh... who?
Slor yo
SteveS oops.  I fell asleep
Slor heh
SteveS hi fres
SteveS Hot cocoa!  BRB...
fres fellas...
Harlock q
Harlock j
SteveS _FujIRC time to backup MyIDE
SteveS _FujIRC q
SteveS damn, it's quiet in here
DogWombl it seems to be quiet in irc channels everywhere
Harlock everyone but us out on friday night
Moloch q
SteveS _FujIRC j
SteveS _FujIRC back
SteveS wb
rrCurtisP q
Woodzy q
DocRotCod j
SteveS _FujIRC q
Velcro_MM q
Velcro_MM j
fres nope
SteveS yup yup yup
SteveS nothing to see here
SteveS Anyone have to travel for Thanksgiving?
SteveS Not me.
DocRotCod q
Urchlay Redb3ard and JellE are driving to Indiana
SteveS oh, is Red from there?
Urchlay think he's from Virginia, but got family there
Redb3ard Grew up in Indiana, only place I'd ever been until I was 18.
Urchlay my grandma (like 88 this year) was going to come up this year
Redb3ard I just happened to be in Virginia, when we started talking.
Urchlay ah
Urchlay grandma can't drive for 4 hours straight (er, way she drives it'd take 8 anyway)
Urchlay the other day my aunt who was gonna do the driving, fell & broke her hip
Urchlay so no aunt, no grandma, no cool cousin
Urchlay (well, maybe cool cousin, she lives in Riverdale IIRC)
SteveS ouch.  well, I wish your aunt a fast recovery.
Urchlay the doc says she's doing great
Urchlay I really don't think of her as *old* but she's in her 60s now I guess
Urchlay (if I ask, she'll say she's still 59 I'm sure)
Urchlay wish I had the car, I'd drive down there
Urchlay go see her in the hospital, bring a fake can of peanuts with a snake in it
Urchlay (a fake one!)
Urchlay eh, sorry, the snake thing is like a 20-year running gag I guess
SteveS um
SteveS ok
