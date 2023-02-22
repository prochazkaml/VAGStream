# Generate data

rm -f Assets.c

xxd -i assets/FONT.TIM >> Assets.c

# Generate header

rm -f Assets.h

grep "=" Assets.c | cut -d " " -f1-3 | while read var; do
	echo "extern $var;" >> Assets.h
done
