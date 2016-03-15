SUBDIRS = . private

linear_includedir = $(includedir)/linear
linear_include_HEADERS = *.h

all-local:
	@( \
		version_file="version.h"; \
		memory_file="memory.h"; \
		if [ -f $${version_file} -a -f $${memory_file} ]; then \
			old_commit_id=`cat $${version_file} | ${AWK} '/LINEAR_COMMIT_ID/{gsub("\"", "", $$3); print $$3}'`; \
		else \
			old_commit_id="nil"; \
		fi; \
		commit_id=`git log --pretty=format:"%H" -1`; \
		if [ $${commit_id} = $${old_commit_id} ]; then \
			echo "repo is not updated: $${commit_id}"; \
			exit 0; \
		fi; \
		cp -f $${version_file}.in $${version_file}; \
		${PERL} -pe "s/\@LINEAR_VERSION_ID\@/@LINEAR_VERSION_ID@/g" -i $${version_file}; \
		${PERL} -pe "s/\@LINEAR_COMMIT_ID\@/$$commit_id/g" -i $${version_file}; \
		cp -f $${memory_file}.in $${memory_file}; \
		${PERL} -pe "s/\@HAVE_STD_SHARED_PTR\@/@HAVE_STD_SHARED_PTR@/g" -i $${memory_file}; \
		${PERL} -pe "s/\@HAVE_TR1_SHARED_PTR\@/@HAVE_TR1_SHARED_PTR@/g" -i $${memory_file}; \
		${PERL} -pe "s/\@HAVE_BOOST_SHARED_PTR\@/@HAVE_BOOST_SHARED_PTR@/g" -i $${memory_file}; \
	)

clean-local:
	rm -f memory.h version.h
	rm -f *~
