Recording Changes
=================

This directory contains "news fragments" which are small files containing text that will be integrated into release notes.
The files can be in restructured text format or plain text.

Each file should be named like ``<JIRA TICKET>.<TYPE>`` with a file extension defining the markup format.
The ``<TYPE>`` should be one of:

* ``feature``: New feature
* ``bugfix``: A bug fix.
* ``api``: An API change.
* ``perf``: A performance enhancement.
* ``doc``: A documentation improvement.
* ``removal``: An API removal or deprecation.
* ``misc``: Changes that are of minor interest.

An example file name would therefore look like ``DM-30291.misc.rst``.

If the change concerns specifically the registry or a datastore the news fragment can be placed in the relevant subdirectory.

You can test how the content will be integrated into the release notes by running ``towncrier --draft --version=V.vv``.
``towncrier`` can be installed from PyPI or conda-forge.
