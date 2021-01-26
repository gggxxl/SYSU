INPUT_FILE_PATH="train_input/alldata.json"
OUTPUT_PATH="train_output/"

hadoop fs -rm -r  $OUTPUT_PATH 

mapred streaming \
    -input $INPUT_FILE_PATH \
    -output $OUTPUT_PATH \
    -mapper "python train_mapper.py" \
    -reducer "python train_reducer.py" \
    -file ./train_mapper.py \
    -file ./train_reducer.py