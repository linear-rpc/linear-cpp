.PHONY: doc

doc:
	@( \
		if [ -z $(DOXYGEN) ]; then \
			echo "please install doxygen and graphvis"; \
		else \
			$(DOXYGEN) Doxyfile; \
		fi \
	)

install-data-local: doc
	@( \
		if [ -e html ]; then \
			mkdir -p $(prefix)/doc/cpp; \
			cp -fr html/* $(prefix)/doc/cpp; \
		fi \
	)

uninstall-local:
	rm -fr $(prefix)/doc/cpp

clean-local:
	rm -fr html
