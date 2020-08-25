#!/bin/bash

#
# This repository already comes with a certificate and private key
# for testing purposes. But you can use or modify this small script
# in order to generate new certificates and keys
#


# Generates a self signed key with 100 years for expiration.
# Used for testing purpose
openssl req -x509 \
        -newkey rsa:4096 \
        -keyout private.key \
        -out cert.pem \
        -days 36500 \
        -nodes \
        -subj "/CN=localhost"
