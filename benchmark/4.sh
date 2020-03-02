if [ "$#" -ne 1 ]; then
        echo "Wrong number of arguments, expected 1."
        exit 1
fi

source test.sh

test_result=$(perform_test_🍆 4 "$1")
user_time=$(get_user_time "$test_result")
used_memory=$(get_used_memory "$test_result")
#echo "$test_result"
echo "$user_time"
echo "$used_memory"

test_result=$(perform_test_c++ 4 "$1")
user_time=$(get_user_time "$test_result")
used_memory=$(get_used_memory "$test_result")
#echo "$test_result"
echo "$user_time"
echo "$used_memory"
