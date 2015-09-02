#!/bin/sh

set -e

CONF="./openssl.cnf"
CA=
KEYLEN=2048
C="JP"
ST="Linear"
O="Linear Sample"
CN=${O}
DAYS=3650 # 10 years

gencacert() {
    CA=$1
    CN_PAD="`echo ${CA} | tr '[a-z]' '[A-Z]'`"
    openssl genrsa -out ${CA}.key ${KEYLEN}
    openssl req -config ${CONF} -new -key ${CA}.key -out ${CA}.csr <<EOF
${C}
${ST}
.
${O}
.
${CN} ${CN_PAD}
.
.
.
EOF
    openssl x509 -req -days ${DAYS} -in ${CA}.csr -signkey ${CA}.key -out ${CA}.pem
}
createca() {
    rm -fr demoCA
    mkdir demoCA
    cd demoCA
    mkdir certs newcerts crt private
    touch index.txt
    echo 00 > serial
    cd ..
}
gencert() {
    TARGET=$1
    CN_PAD="`echo ${CA} | tr '[a-z]' '[A-Z]'`"
    openssl genrsa -out ${TARGET}.key ${KEYLEN}
    openssl req -new -key ${TARGET}.key -out ${TARGET}.csr <<EOF
${C}
${ST}
.
${O}
.
${CN} ${CN_PAD}
.
.
.
EOF
    openssl ca -days ${DAYS} -in ${TARGET}.csr -keyfile ${CA}.key -cert ${CA}.pem -out ${TARGET}.pem<<EOF
y
y
EOF
}

# gen CA
gencacert ca
createca
# gen server
gencert server
# gen client
gencert client
