package HomeBank;

use warnings FATAL => 'all';
use strict;

use Symbol qw/delete_package/;

=head1 NAME

HomeBank - Perl plugin bindings for C<homebank>

=head1 SYNOPSIS

    # NAME: Example Plugin

    sub new {
        my $class = shift;
        my $self  = $class->SUPER::new(@_);

        $self->on(
            terminate => sub {
                print "Terminating...\n";
            },
        );

        $self;
    }

    sub on_unhandled {
        my ($self, $hook_id) = @_;
        print "An unhandled hook named '$hook_id' was called.\n";
    }

=head1 DESCRIPTION

The C<HomeBank> class provides the infrastructure for loading plugins and handling the registration and calling of
hooks.

=head1 VARIABLES

=head2 %plugins

Contains all of the information about each loaded perl plugin. Plugins probably shouldn't mess around with this.

=cut

our %plugins;

=head1 METHODS

=head2 load_plugin $filepath

Load a plugin with the given name. Dies if a plugin with the given name cannot be found or if the plugin couldn't
successfully be eval'd. L<homebank> calls this to load enabled plugins; plugins themselves probably shouldn't ever use
this.

=cut

sub load_plugin {
    my $filepath = shift;

    my $package = _valid_package_name($filepath);
    $plugins{$package} ||= {};

    my $mtime = -M $filepath;
    if (defined $plugins{$package}->{mtime} && $plugins{$package}->{mtime} <= $mtime) {
        warn "Already loaded $filepath";
    } else {
        delete_package $package if exists $plugins{$package}->{mtime};

        open my $fh, $filepath or die "Open '$filepath' failed ($!)";
        binmode $fh, 'utf8';
        local $/ = undef;
        my $code = <$fh>;
        close $fh;

        my $eval = qq/# line 1 "$filepath"\npackage $package; use base 'HomeBank::Plugin'; $code/;
        {
            my (%plugins, $mtime, $package);
            eval "$eval; 1" or die $@;
        }

        $plugins{$package}->{mtime} = $mtime;
    }
    if (!exists $plugins{$package}->{instance}) {
        $plugins{$package}->{instance} = $package->new or die "Plugin instantiation failed";
    }
}

=head2 unload_plugin $filepath

The opposite of L<load_plugin>.

=cut

sub unload_plugin {
    my $filepath = shift;
    my $package  = _valid_package_name($filepath);

    return unless exists $plugins{$package};

    if ($package->can('delete_package_on_unload') && $package->delete_package_on_unload) {
        delete $plugins{$package};
        delete_package $package;
    } else {
        delete $plugins{$package}->{instance};
        delete $plugins{$package}->{hooks};
    }
}

=head2 execute_action $filepath

Allow the plugin specified by C<$filepath> to perform an action. This is called when the plugin is "activated" by the
user. Most plugins should run a modal dialog to allow the user to see and edit plugin preferences.

=cut

sub execute_action {
    my $filepath = shift;
    my $package  = _valid_package_name($filepath);

    return unless exists $plugins{$package};

    my $instance = $plugins{$package}->{instance};
    $instance->EXECUTE if $instance && $instance->can('EXECUTE');
}

=head2 read_metadata $filepath

Get the metadata for a plugin without evaluating it. Plugin metadata should be in the first 100 lines of the plugin file
and should look something like this:

    # NAME:     Foobar
    # VERSION:  0.01
    # ABSTRACT: This plugin does something.
    # AUTHOR:   John Doe <jdoe@acme.tld>
    # WEBSITE:  http://acme.tld/

=cut

sub read_metadata {
    my $filepath = shift;

    my $package  = _valid_package_name($filepath);
    $plugins{$package} ||= {};

    return $plugins{$package}->{metadata} if exists $plugins{$package}->{metadata};

    my @keywords = qw/name version abstract author website/;
    my $keywords = join('|', @keywords);

    my $metadata = {};
    open my $fh, $filepath or die "Open '$filepath' failed ($!)";
    my $count = 0;
    for my $line (<$fh>) {
        last if 100 < ++$count;
        my ($key, $val) = $line =~ /^#[ \t]*($keywords)[ \t]*[=:](.*)/i;
        if ($key && $val) {
            $val =~ s/^\s*//;
            $val =~ s/\s*$//;
            $metadata->{lc $key} = $val;
        }
    }
    close $fh;

    $plugins{$package}->{metadata} = $metadata;
}

=head2 call_hook $hook_id, ...

Invoke each perl plugins' hook handlers for the given hook. Additional arguments are passed through to each handler.
Plugins shouldn't use this.

=cut

sub call_hook {
    my $hook = shift;

    $hook =~ s/[.-]/_/g;

    for my $package (keys %plugins) {
        my $hooks = ($plugins{$package} ||= {})->{hooks} ||= {};
        my $count = 0;
        for my $cb (@{$hooks->{$hook} ||= []}) {
            eval { $cb->(@_); 1 } or warn $@;
            $count++;
        }
        if ($count == 0) {
            for my $cb (@{$hooks->{unhandled} ||= []}) {
                eval { $cb->($hook, @_); 1 } or warn $@;
            }
        }
    }
}

=head2 register_method_hooks $plugin

Register hooks defined as methods that begin with `on_'.

=cut

sub register_method_hooks {
    my $plugin = shift;
    my $package = ref $plugin;

    no strict 'refs';
    my %subs = map { $_ =~ /^on_(.+)/ ? ($1 => $_) : () } keys %{"${package}::"};
    use strict 'refs';

    register_hooks($plugin, %subs);
}

=head2 register_hooks $plugin, %hooks

Register hooks for a plugin.

=cut

sub register_hooks {
    my ($plugin, %hooks) = @_;
    my $package = ref $plugin;

    my $hooks = ($plugins{$package} ||= {})->{hooks} ||= {};
    for my $hook (keys %hooks) {
        if (!ref($hooks{$hook}) && defined &{"${package}::$hooks{$hook}"}) {
            push @{$hooks->{$hook} ||= []}, sub { unshift @_, $plugin; goto &{"${package}::$hooks{$hook}"} };
        } elsif (ref($hooks{$hook}) eq 'CODE') {
            push @{$hooks->{$hook} ||= []}, $hooks{$hook};
        } else {
            warn "Hook callback is unusable";
        }
    }
}

=head2 unregister_hooks $package, [@hooks]

Unregister hooks for a package. If no hooks are specified, B<all> hooks will be unregistered.

=cut

sub unregister_hooks {
    my ($package, @hooks) = @_;

    if (@hooks) {
        for my $hook (@hooks) {
            (($plugins{$package} ||= {})->{hooks} ||= {})->{$hook} = [];
        }
    } else {
        ($plugins{$package} ||= {})->{hooks} = {};
    }
}

=head2 _valid_package_name $string

Turn a string into a valid name of a package.

=cut

sub _valid_package_name {
    my $str = shift;
    $str =~ s|.*?([^/\\]+)\.pl$|$1|;
    $str =~ s|([^A-Za-z0-9\/_])|sprintf("_%2x",unpack("C",$1))|eg;
    $str =~ s|/(\d)|sprintf("/_%2x",unpack("C",$1))|eg;
    $str =~ s|[/_]|::|g;
    "HomeBank::Plugin::$str";
}


package HomeBank::Boolean;

use overload
    '0+'        => sub { ${$_[0]} },
    '++'        => sub { $_[0] = ${$_[0]} + 1 },
    '--'        => sub { $_[0] = ${$_[0]} - 1 },
    fallback    => 1;

package Types::Serialiser::Boolean;
@HomeBank::Boolean::ISA = Types::Serialiser::Boolean::;


package HomeBank::Plugin;

sub new {
    my ($class, $self) = (shift, shift || {});
    bless $self, $class;
    HomeBank::register_method_hooks($self);
    $self;
}

sub on {
    goto &HomeBank::register_hooks;
}

sub off {
    goto &HomeBank::unregister_hooks;
}


package HomeBank::Transaction;

sub datetime {
    require DateTime;
    require DateTime::Format::Strptime;
    my $dt = DateTime->new(shift->date);
    $dt->set_formatter(DateTime::Format::Strptime->new(pattern => '%Y-%m-%d'));
    $dt;
}


=head1 AUTHOR

Charles McGarvey <chazmcgarvey@brokenzipper.com>

=head1 COPYRIGHT AND LICENSE

This software is copyright (c) 2013 Charles McGarvey.

This file is part of HomeBank.

HomeBank is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

HomeBank is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

=cut

1;
