
# NAME:     Transfer Matcher
# VERSION:  0.01
# ABSTRACT: Automatically find and pair together internal transfers.
# AUTHOR:   Charles McGarvey <chazmcgarvey@brokenzipper.com>
# WEBSITE:  http://www.homebank.free.fr/

eval { HomeBank->version } or die "Cannot run outside of HomeBank";

use warnings FATAL => 'all';
use strict;

my $days = 3;

sub on_transaction_inserted {
    my ($self, $txn) = @_;

    my @match = grep {
        $txn->account_num != $_->account_num &&
        $txn->amount == -$_->amount &&
        abs($txn->date - $_->date) <= $days
    } HomeBank::File->transactions;

    return unless @match;

    $txn->pair_with(@match);
}

