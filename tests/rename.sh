# Rename all *.txt to *.text
for f in *.fun; do
    mv -- "$f" "${f%.fun}.pi"
done
