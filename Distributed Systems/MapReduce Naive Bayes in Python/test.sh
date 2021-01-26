INPUT_FILE_PATH="train_output/part-00000"
OUTPUT_PATH="test_output/"

hadoop fs -rm -r  $OUTPUT_PATH 

mapred streaming \
    -input $INPUT_FILE_PATH \
    -output $OUTPUT_PATH \
    -mapper "python test_mapper.py" \
    -reducer "python test_reducer.py" \
    -file ./test_mapper.py \
    -file ./test_reducer.py