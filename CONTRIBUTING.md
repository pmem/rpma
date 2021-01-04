# Contributing to the Remote Persistent Memory Access (RPMA) library (librpma)

Down below you'll find instructions on how to contribute to the librpma library.

Your contributions are most welcome!  You'll find it is best to begin
with a conversation about your changes, rather than just writing a bunch
of code and contributing it out of the blue.
To propose new features, suggest adding features, or simply start a dialogue about librpma,
open an issue in our [GitHub Issues Database](https://github.com/pmem/rpma/issues)

**Note**: If you do decide to implement code changes and contribute them,
please make sure you agree your contribution can be made available
under the [BSD-style License used for the RPMA](https://github.com/pmem/rpma/blob/master/LICENSE).

**Note**: Submitting your changes also means that you certify the following:

```
Developer's Certificate of Origin 1.1

By making a contribution to this project, I certify that:

(a) The contribution was created in whole or in part by me and I
    have the right to submit it under the open source license
    indicated in the file; or

(b) The contribution is based upon previous work that, to the best
    of my knowledge, is covered under an appropriate open source
    license and I have the right under that license to submit that
    work with modifications, whether created in whole or in part
    by me, under the same open source license (unless I am
    permitted to submit under a different license), as indicated
    in the file; or

(c) The contribution was provided directly to me by some other
    person who certified (a), (b) or (c) and I have not modified
    it.

(d) I understand and agree that this project and the contribution
    are public and that a record of the contribution (including all
    personal information I submit with it, including my sign-off) is
    maintained indefinitely and may be redistributed consistent with
    this project or the open source license(s) involved.
```

In case of any doubt, the gatekeeper may ask you to certify the above in writing,
i.e. via email or by including a `Signed-off-by:` line at the bottom
of your commit comments.

To improve tracking of who is the author of the contribution, we kindly ask you
to use your real name (not an alias) when committing your changes to the RPMA:
```
Author: Random J Developer <random@developer.example.org>
```

## Code contributions

### Code formatting

Here is the list of the most important rules:

- The limit of line length is 80 characters.
- Indent the code with TABs, not spaces. Tab width is 8 characters.
- Do not break user-visible strings (even when they are longer than 80 characters), but make them the only token in the line.
- Put each variable declaration in a separate line.
- Do not use C++ comments `//` (except for the License).
- Spaces around operators are mandatory.
- No whitespace is allowed at the end of line.
- For multi-line macros, do not put whitespace before `\` character.
- Precede definition of each function with a brief, non-trivial description. (Usually a single line is enough.)
- Use `XXX` tag to indicate a hack, problematic code, or something to be done.
- For pointer variables, place the `*` close to the variable name not pointer type.
- Avoid unnecessary variable initialization.
- Never type `unsigned int` - just use `unsigned` in such case. Same with `long int` and `long`, etc.
- Sized types like `uint32_t`, `int64_t` should be used when there is an on-media format. Otherwise, just use `unsigned`, `long`, etc.
- Functions with local scope must be declared as `static`.

Before contributing please remember to run:
```shell
	$ make cstyle
	$ make check-whitespace
```

This will check all C files in the tree for style issues.

### Commit messages

All commit lines (entered when you run `git commit`) must follow the common conventions for git commit messages:

- The first line is a short summary, no longer than **50 characters**, starting with an area name and then a colon. There should be no period after the short summary.
- Valid area names are: **rpma, test, examples, doc** and **common** (for everything else).
- It is acceptable for the short summary to be the only thing in the commit message if it is a trivial change. Otherwise, the second line must be a blank line.
- Starting at the third line, additional information is given in complete English sentences and, optionally, bulleted points. This content must not extend beyond **column 72**.
- The English sentences should be written in the imperative, so you say "Fix bug X" instead of "Fixed bug X" or "Fixes bug X".
- Bullet points should use hanging indents when they take up more than one line (see example below).
- There can be any number of paragraphs, separated by a blank line, as many as it takes to describe the change.
- Any references to GitHub issues are at the end of the commit message.

If you want to check the commit before creating PR, run:
```shell
	$ make check-commits
```

For example, here is a properly-formatted commit message:
```
doc: fix code formatting in man pages

This section contains paragraph style text with complete English
sentences.  There can be as many paragraphs as necessary.

- Bullet points are typically sentence fragments

- The first word of the bullet point is usually capitalized and
  if the point is long, it is continued with a hanging indent

- The sentence fragments don't typically end with a period

Ref: rpma/issues#1
```

## Bug reports

Bugs for the RPMA project are tracked in our
[GitHub Issues Database](https://github.com/pmem/rpma/issues).

When reporting a new bug, please use `New issue` button.
Provide as much information as possible.

## Other issues

On our issues page we also gather feature requests and questions. Templates to use
are `Feature` and `Question`, respectively.  They should help deliver a meaningful
description of a feature or ask a question to us.
