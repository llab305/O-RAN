# Python xapp frame
Ping-Pong is a test for sending messages

xapp_example_tlv is an xapp example rewritten based on ping-pong that also need routing table to active

# Running locally for ping-pong

Running the two examples (adjust for your shell notation)

    pip install --user -e .
    cd examples
    set -x LD_LIBRARY_PATH /usr/local/lib/:/usr/local/lib64; set -x  RMR_SEED_RT test_route.rt; python pong_xapp.py
    (diff tmux window)
    set -x LD_LIBRARY_PATH /usr/local/lib/:/usr/local/lib64; set -x  RMR_SEED_RT test_route.rt; python ping_xapp.py

# Running in Docker for ping-pong

    docker build -t ping:latest -f  Dockerfile-Ping .
    docker build -t pong:latest -f  Dockerfile-Pong .
    docker run -i --net=host ping:latest
    docker run -i --net=host pong:latest
    
# Running in Docker for xapp_example_tlv
    
    docker build --build-arg containerIP=127.0.0.1 -t xapp_tlv:release -f  Dockerfile-xapp_tlv  .
    sudo docker run -i --net=host xapp_tlv:release
