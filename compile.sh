
if [[ $CLANG == "" ]] ; then
	CLANG="clang-15"
fi

echo "using clang: $CLANG"

eval "$CLANG -std=c17 -Wall -Wextra -Wpedantic -o uber uber.c"
