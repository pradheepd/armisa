# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.armisa.Debug:
/Users/pradeep/Documents/IOT_proj/ARMISA/XCode/Debug/armisa:
	/bin/rm -f /Users/pradeep/Documents/IOT_proj/ARMISA/XCode/Debug/armisa


PostBuild.armisa.Release:
/Users/pradeep/Documents/IOT_proj/ARMISA/XCode/Release/armisa:
	/bin/rm -f /Users/pradeep/Documents/IOT_proj/ARMISA/XCode/Release/armisa


PostBuild.armisa.MinSizeRel:
/Users/pradeep/Documents/IOT_proj/ARMISA/XCode/MinSizeRel/armisa:
	/bin/rm -f /Users/pradeep/Documents/IOT_proj/ARMISA/XCode/MinSizeRel/armisa


PostBuild.armisa.RelWithDebInfo:
/Users/pradeep/Documents/IOT_proj/ARMISA/XCode/RelWithDebInfo/armisa:
	/bin/rm -f /Users/pradeep/Documents/IOT_proj/ARMISA/XCode/RelWithDebInfo/armisa




# For each target create a dummy ruleso the target does not have to exist
