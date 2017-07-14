make
rm bbr.txt
make test > bbrtest.txt
cat bbrtest.txt
python plot.py bbrtest.txt

