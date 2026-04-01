import pathlib
import re
import subprocess
import sys

docsDirectory = pathlib.Path(__file__).parent / "docs"

# Pandoc escapes [ and ] in plain text, turning [text](url) into \[text\](url).
# This pattern matches that escaped form and restores it as a proper markdown link.
escapedLinkPattern = re.compile(r'\\\[([^\]]*)\\\]\(([^)]*)\)')

# Pandoc escapes dashes and adds blank lines inside frontmatter blocks, turning
#   ---          into   \-\--
#   icon: x             (blank line)
#   ---                 icon: x
#                       (blank line)
#                       \-\--
# \A anchors to the very start of the file so this only affects the frontmatter header.
escapedFrontmatterPattern = re.compile(r'\A(?:\\-|-){3,}\n\n((?:[\w-]+: [^\n]+\n\n)+)(?:\\-|-){3,}')

docxFiles = list(docsDirectory.rglob("*.docx"))
totalCount = len(docxFiles)

successCount = 0
failureCount = 0

for fileIndex, docxFile in enumerate(docxFiles):
	print("{} of {}".format(fileIndex + 1, totalCount), end = "\r", flush = True)
	outputFile = docxFile.with_suffix(".md")
	result = subprocess.run(
		["pandoc", str(docxFile), "-o", str(outputFile), "--wrap=none"],
		capture_output = True,
		text = True
	)
	if result.returncode == 0:
		markdownText = outputFile.read_text(encoding = "utf-8")
		restoredText = escapedLinkPattern.sub(r'[\1](\2)', markdownText)
		restoredText = escapedFrontmatterPattern.sub(
			lambda match: "---\n" + match.group(1).replace("\n\n", "\n").rstrip("\n") + "\n---",
			restoredText
		)
		if restoredText != markdownText:
			outputFile.write_text(restoredText, encoding = "utf-8")
