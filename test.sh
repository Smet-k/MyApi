PORT=${1:-8080}

for i in {1..1000}; do
    curl http://localhost:$PORT & 
done

wait