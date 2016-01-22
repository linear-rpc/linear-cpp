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
    PASS=$2
    CN_PAD="`echo ${TARGET} | tr '[a-z]' '[A-Z]'`"
    if [ "x${PASS}" != "x" ]; then
        openssl genrsa -aes128 -passout pass:${PASS} -out ${TARGET}.key ${KEYLEN}
        openssl req -new -key ${TARGET}.key -passin pass:${PASS} -out ${TARGET}.csr <<EOF
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
    else
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
    fi
    openssl ca -days ${DAYS} -in ${TARGET}.csr -keyfile ${CA}.key -cert ${CA}.pem -out ${TARGET}.pem<<EOF
y
y
EOF
}
p2dkey() {
    openssl rsa -in $1.key -inform PEM -out $1.key.der -outform DER
}
p2dcert() {
    openssl x509 -in $1.pem -inform PEM -out $1.der -outform DER
}

CA_IDENT="ca"
SERVER_IDENT="server"
CLIENT_IDENT="client"

# gen CA
gencacert ${CA_IDENT}
createca
# gen server
gencert ${SERVER_IDENT}
# gen client
gencert ${CLIENT_IDENT}
# convert to der
p2dkey ${CLIENT_IDENT}
p2dkey ${SERVER_IDENT}
p2dcert ${CA_IDENT}
p2dcert ${SERVER_IDENT}
p2dcert ${CLIENT_IDENT}

PASSPHRASE=passphrase

# gen server cert with passphrase
gencert ${SERVER_IDENT}-w-pass ${PASSPHRASE}
# gen client cert with passphrase
gencert ${CLIENT_IDENT}-w-pass ${PASSPHRASE}
