
# NAME:     Hello World
# VERSION:  0.01
# ABSTRACT: This is the "hello world" of HomeBank plugins.
# AUTHOR:   Charles McGarvey <chazmcgarvey@brokenzipper.com>
# WEBSITE:  http://acme.tld/
# (These comments are read, before the plugin is executed, to provide some
# information to HomeBank and the user about what this plugin is.)

eval { HomeBank->version } or die "Cannot run outside of HomeBank";

use warnings;
use strict;

use Scalar::Util qw/weaken/;

#use Moose;

#has "cool_beans",
    #is      => 'rw',
    #isa     => 'Str',
    #lazy    => 1,
    #default => "Booya!!!";


our $counter = 0;
our $temp;

my $ACC;

sub new {
    my $class = shift;
    my $self  = $class->SUPER::new(@_);

    $self->on(account_inserted => sub {
            my $acc = shift;
            print "account inserted: ", Dumper($acc);
            print "account name is ", $acc->name, " and balance is ", $acc->bank_balance, "\n";
            #$acc->name("FOOOOBAR!");
            if ($acc->name eq 'Vacation') {
                $acc->remove;
                $ACC = $acc;
            }
            print Dumper($acc->is_inserted);
            if ($acc->is_inserted) {
                print "IT IS INSERTED\n";
            } else {
                print "not inserted\n";
            }
            print Dumper($acc->transactions);
        });

    #print $self->cool_beans, "\n";
    #$self->cool_beans(123);
    #print $self->cool_beans, "\n";

    $self->create_menuitem;

    $self;
}

sub on_create_main_window {
    my $self = shift;
    my $window = shift;

    if (!$window) {
        require Gtk2;
        $window = HomeBank->main_window;
    }

    Dump($window);
    print Dumper($window);
    $window->set_title("foo bar baz");
    print $window->get_title, "\n";

    HomeBank->hook("my_hook", $window);

    $self->create_menuitem;
}

my $test_win;

sub on_test {
    my $self = shift;
    require Gtk2;

    my $window = Gtk2::Window->new('toplevel');
    use Devel::Peek;
    Dump($window);
    print Dumper($window);
    $window->set_title("Hello World");
    #$window->signal_connect(delete_event => sub { Gtk2->main_quit });
    $window->signal_connect(delete_event => sub { undef $test_win });

    my $button = Gtk2::Button->new('Click Me!');
    Dump($button);
    print Dumper($button);
    $button->signal_connect(clicked => sub {
            print "Hello Gtk2-Perl: $counter (perl plugin: $self)\n";
            $counter++;
            #if ($temp->is_inserted) {
                #print "$temp is inserted\n";
            #} else {
                #print "$temp is NOT inserted\n";
            #}
            #if ($counter == 5) {
                #$temp = undef;
            #}
    my $acc = HomeBank::Account->get(rand(10));
    print "Changin account named ", $acc->name, " to ", $acc->name($acc), "\n";
    HomeBank->main_window->queue_draw;

        });
    $window->add($button);

    $window->show_all;
    $test_win = $window;

    weaken $self;
}

sub on_enter_main_loop {
    my $self = shift;

    use Data::Dumper;
    print Dumper(\@_);
    my $t = HomeBank::Transaction->new;
    print "Transaction::::::::  $t: ", $t->amount, "\n";

    $temp = HomeBank::Account->get(7);
    print "retained account: ", $temp->name, "\n";

    #require Gtk2;
    #
    my $txn = HomeBank::Transaction->new;
    $txn->amount(12.3456);
    print Dumper($txn), $txn->amount, "\n";
    #$txn->open;

    my @ret = HomeBank->hook("my_hook", @_, $temp, [qw/foo bar baz/, $txn], { asf => 42, quux => \1, meh => HomeBank->main_window });
    #my @ret = HomeBank->hook("my_hook", @_, HomeBank->main_window, {
    #foo => 'bar', baz => 42
    #});
    print Dumper(\@ret);

    print "adding back account...\n";
    $ACC->name("vacation with a different name");
    $ACC->insert;
    HomeBank::Account->compute_balances;
    print "account name is ", $ACC->name, " and balance is ", $ACC->balance, "\n";
    print Dumper($ACC->transactions);

    my $cloned = $ACC->clone;
    $cloned->name("vacation copy");
    $cloned->insert;
    #my $asdf = $cloned->open;
    #$asdf->set_title("this is a new friggin account");

    #my $z = HomeBank::Account->get_by_name('Checking');
    for my $xc (HomeBank::File->transactions) {
        use DateTime;
        my $num = $xc->date;
        my $date = DateTime->new($xc->date)->datetime;
        print "transaction of amount: ", $xc->amount, "\t", $xc->wording, ", ", $xc->info, ", $num, $date\n";
    }

    HomeBank::File->owner('Billy Murphy');
    #HomeBank::File->anonymize;
    print HomeBank::File->owner, "\n";

    HomeBank::File->baz($ACC);
}

sub on_deep_hook_recursion {
    my $self = shift;
    my $level = shift;
    print STDERR "recursion is too deep ($level)\n";
    exit -2;
}

sub on_my_hook {
    my $self = shift;
    print "This is MY HOOK!!!!!!\n";
    print Dumper(\@_);

    print Dumper($_[2]);
    Dump($_[2]);
    if ($_[2]) {
        print "meh\n";
    }
    if ($_[2]->isa('HomeBank::Boolean')) {
        print "it is a home;;boolean\n";
    }
    if ($_[2]->isa('Types::Serialiser::Boolean')) {
        print "it is a types serialiser thingy\n";
    }
    if ($_[2]->isa('HomeBank::BooleanBase')) {
        print "it is a base bool\n";
    }

    my $win = $_[6];
    if ($win && ref($win) eq 'HASH') {
        my $w = $win->{meh};
        if ($w) {
            $w->set_title("this is MY HOOK setting a window title");
        }
    }
    #print Dumper($acc);
    #print "transferred account: ", $acc->name, "\n";

    #my $fff = HomeBank::File->foo({foo => 'asdf', bar => 123456789});
    my $fff = HomeBank::File->meh([qw/hello this is a test 82/, \1, {foo => 'bar'}, 48]);
    print Dumper($fff);

    print "my hook done\n";
}

sub on_unhandled {
    my ($self, $hook) = @_;
    warn "Unhandled hook '$hook'\n";
    #HomeBank->warn($hook, 'Hook not handled.');
}

sub on_main_window_disposal {
    my $self = shift;
    print "main window disposed so forgetting about merge id et al.\n";
    delete $self->{merge_id};
    delete $self->{action_group};
}

sub DESTROY {
    my $self = shift;
    print "DESTROYING HELLO WORLD!!!!!!\n";
    if ($test_win) {
        print "there is a test_win...\n";
    }
    $test_win->destroy if $test_win;

    $self->destroy_menuitem;
}

sub destroy_menuitem {
    my $self = shift;

    return unless $self->{merge_id};

    my $ui_manager = HomeBank->main_ui_manager;
    $ui_manager->remove_action_group($self->{action_group});
    $ui_manager->remove_ui($self->{merge_id});
}

sub create_menuitem {
    my $self = shift;

    return if $self->{merge_id};

    require Gtk2;

    my $ui_manager = HomeBank->main_ui_manager;
    print Dumper($ui_manager);
    return unless $ui_manager;

    $self->{merge_id} = $ui_manager->new_merge_id;
    $self->{action_group} = Gtk2::ActionGroup->new('HelloActionGroup');

    my $action = Gtk2::Action->new(name => 'HelloPlugin', label => 'Booyah!', stock_id => 'prf-plugins', tooltip => 'blaaaargh');
    $action->signal_connect(activate => sub { print "hello!!!!!!!!\n" });
    $self->{action_group}->add_action($action);

    $ui_manager->insert_action_group($self->{action_group}, -1);
    $ui_manager->add_ui($self->{merge_id}, 'ui/MenuBar/PluginMenu', 'HelloPlugin', 'HelloPlugin', 'auto', '');
    #$self->{merge_id} = $ui_manager->new_merge_id;
    $ui_manager->add_ui($self->{merge_id}, 'ui/ToolBar', 'HelloPluginTool', 'HelloPlugin', 'auto', '');
}

sub EXECUTE {
    print "the perl plugin is being configured.....\n";
    HomeBank->info("Hello Prefs", "YEEEEEARGGH!!!!!");
}

#__PACKAGE__->meta->make_immutable;
