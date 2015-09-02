SUBDIRS = . private

linear_includedir = $(includedir)/linear
linear_include_HEADERS = *.h

## create version.h, memory.h
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
		commit_date=`git log --pretty=format:"%ad" -1`; \
		${PERL} -pe "s/VersionId/\"@LINEAR_VERSION_ID@\"/g" -i $${version_file}; \
		${PERL} -pe "s/CommitId/\"$$commit_id\"/g" -i $${version_file}; \
		${PERL} -pe "s/CommitDate/\"$$commit_date\"/g" -i $${version_file}; \
		cp -f $${memory_file}.in $${memory_file}; \
		if [ @HAVE_STD_SHARED_PTR@ ]; then \
			${PERL} -pe "s/SharedPtrDefinition/#define HAVE_STD_SHARED_PTR 1/g" -i $${memory_file}; \
		elif [ @HAVE_TR1_SHARED_PTR@ ]; then \
			${PERL} -pe "s/SharedPtrDefinition/#define HAVE_TR1_SHARED_PTR 1/g" -i $${memory_file}; \
		elif [ @HAVE_BOOST_SHARED_PTR@ ]; then \
			${PERL} -pe "s/SharedPtrDefinition/#define HAVE_BOOST_SHARED_PTR 1/g" -i $${memory_file}; \
		else \
			${PERL} -pe "s/SharedPtrDefinition/#define NOT_EXISTS_SHARED_PTR/g" -i $${memory_file}; \
		fi \
	)

clean-local:
	rm -f memory.h version.h
	rm -f *~
