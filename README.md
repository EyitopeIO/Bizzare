# Bizzare
Given a device (IP address) on a network, predict how much bandwidth it needs

## How Bizzare does its thing

- Get IP address from user
- Within a time period, sum the bytes it has sent and received
- Train model with data and save it in SQL database

## TODO
- nfct_query may block for long. Mitigate this
- On forwarding, dest. IP in reply is not src. IP, so counting is wrong