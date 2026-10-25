# The documentation of SLUDGE, and the example projects.

install(FILES SLUDGE_Help.html Credits.html DESTINATION ${SLUDGE_DOCDIR})
install(DIRECTORY SLUDGEDevKitHelp SLUDGELinuxHelp ExampleProjects
	DESTINATION ${SLUDGE_DOCDIR}
	# The help index is for the help viewer of Mac OS X, and the example
	# projects play the .xm versions of their music.
	PATTERN "*.helpindex" EXCLUDE
	PATTERN "*.mod" EXCLUDE)
