use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
use Config;

$osname=$Config{osname};

print "Configuring for $osname ...\n";

if ($osname eq 'solaris') {
	$ex_cc_flags='-symbolic';
}


WriteMakefile(
    'NAME'	=> 'Authen::TacacsPlus',
    'VERSION_FROM' => 'TacacsPlus.pm', # finds $VERSION
    'LIBS'	=> [''],   # e.g., '-lm' 
    'INC'	=> '',     # e.g., '-I/usr/include/other' 
    'MYEXTLIB' => "$ex_cc_flags $defines{$osname}".'  tacpluslib/libtacplus$(LIB_EXT)',
    'DISTNAME'          => 'TacacsPlus',
    'dist'                  => { COMPRESS => 'gzip', SUFFIX => 'gz' },
);
sub MY::postamble {         
'                           
$(MYEXTLIB): tacpluslib/Makefile 
	cd tacpluslib && $(MAKE) 
';                          
}                           
