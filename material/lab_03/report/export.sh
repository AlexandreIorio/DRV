#!/bin/bash

# Vérification des arguments
if [ $# -ne 1 ]; then
    echo "Usage: $0 <fichier.md>"
    exit 1
fi

# Nom du fichier Markdown
input_file="$1"
base_name=$(basename "$input_file" .md)

# Génération du fichier HTML
pandoc "$input_file" -o "${base_name}.html" --template=template.html --from=markdown --to=html --css=template.css --toc

# Génération du fichier PDF
pandoc "$input_file" -o "${base_name}.pdf" --template=template.html --from=markdown --to=html --css=template.css --pdf-engine=weasyprint --highlight-style=pygments --toc

echo "Les fichiers ${base_name}.html et ${base_name}.pdf ont été générés avec succès."
