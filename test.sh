# while getopts "a:b:cdef" opt; do
#   case $opt in
#     a)
#       echo "this is -a the arg is ! $OPTARG" 
#       ;;
#     b)
#       echo "this is -b the arg is ! $OPTARG" 
#       ;;
#     c)
#       echo "this is -c the arg is ! $OPTARG" 
#       ;;
#     \?)
#       echo "Invalid option: -$OPTARG" 
#       ;;
#   esac
# done

cleos="cleos --url http://127.0.0.1:18888 --wallet-url http://127.0.0.1:8888 "
START_SECOND=`date +%s`
for((i=10000;i<20000;i++))
do
  nohup $cleos push action kqjs charge "[\"$i\",\"13699223216\", \"200\", \"10000001\", \"10000002\", \"0.2\", \"`date +%s`\"]" -p kqjs &
done
END_SECOND=`date +%s`
DURATION=`expr $END_SECOND - $START_SECOND`
echo "运行时长:"  $DURATION





