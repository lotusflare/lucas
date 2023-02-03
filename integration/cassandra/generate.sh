#!/bin/sh

# https://docs-previous.pega.com/decision-management/87/creating-java-keystores-and-truststores-cassandra-encryption

# keytool -genkey -keyalg RSA -alias shared -validity 36500 -keystore keystore.shared -storepass cassandra -keypass cassandra -dname "CN=None, OU=None, O=None, L=None, C=None"

# keytool -export -alias shared -file shared.cer -keystore keystore.shared -storepass cassandra

# keytool -importcert -v -trustcacerts -noprompt -alias shared -file shared.cer -keystore truststore.shared -storepass cassandra


# https://docs.datastax.com/eol/en/security/6.0/security/secSetUpSSLCert.html#secByoRootCa

openssl req -config rootca.conf -new -x509 -nodes -keyout rootca.key -out rootca.crt -days 3650

keytool -keystore dse-truststore.jks -storetype JKS -importcert -file 'rootca.crt' -keypass cassandra -storepass cassandra -alias rootca_name -noprompt

# verify
keytool -list -keystore dse-truststore.jks -storepass cassandra

keytool -genkeypair \
-keyalg RSA \
-alias node_name \
-keystore node-keystore.jks \
-storepass cassandra \
-keypass cassandra \
-keysize 2048 \
-dname "CN=node_name, OU=cluster_name, O=org_name, C=CC"

# verify
keytool -list \
-keystore node-keystore.jks \
-storepass cassandra

# generate csr
keytool -keystore node-keystore.jks \
-alias node_name \
-certreq -file signing_request.csr \
-keypass cassandra \
-storepass cassandra

# signing
openssl x509 -req -CA 'rootca.crt' \
-CAkey 'rootca.key' \
-in signing_request.csr \
-out signing_request.crt_signed \
-CAcreateserial

# import
keytool -keystore node-keystore.jks \
-alias rootca_name \
-importcert -file 'rootca.crt' \
-keypass cassandra \
-storepass cassandra \
-noprompt


keytool -keystore node-keystore.jks \
-alias node_name \
-importcert -file signing_request.crt_signed \
-keypass cassandra \
-storepass cassandra \
-noprompt

# client key
openssl req -newkey rsa:2048 \
-nodes \
-keyout client_key.key \
-out client_signing_request.csr \
-config client.conf

# sign it
openssl x509 -req -CA 'rootca.crt' \
-CAkey 'rootca.key' \
-in signing_request.csr \
-out client_cert.crt_signed \
-CAcreateserial \
-passin pass:cassandra


# client cert: client_cert.crt_signed
# client key: client_key.key
