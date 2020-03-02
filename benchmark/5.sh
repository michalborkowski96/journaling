if [ "$#" -ne 2 ]; then
        echo "Wrong number of arguments, expected 2."
        exit 1
fi

source test.sh

test_result=$(perform_test_🍆 5 "$1 $2")
user_time=$(get_user_time "$test_result")
used_memory=$(get_used_memory "$test_result")
#echo "$test_result"
echo "$user_time"
echo "$used_memory"

test_result=$(perform_test_c++ 5 "$1 $2")
user_time=$(get_user_time "$test_result")
used_memory=$(get_used_memory "$test_result")
#echo "$test_result"
echo "$user_time"
echo "$used_memory"
