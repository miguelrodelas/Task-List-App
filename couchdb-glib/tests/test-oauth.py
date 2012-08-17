#!/usr/bin/python

import sys
from oauth import oauth

(consumer_key, consumer_secret, actual_token, token_secret, url,
	method, nonce, timestamp) = sys.argv[1:]

consumer=oauth.OAuthConsumer(consumer_key, consumer_secret)
token=oauth.OAuthToken(actual_token, token_secret);
rq=oauth.OAuthRequest.from_consumer_and_token(consumer, token=token,
	http_method=method, http_url=url,
        parameters={'oauth_nonce':nonce,'oauth_timestamp':timestamp});
parameters = {
    'oauth_consumer_key': consumer_key,
    'oauth_nonce': nonce,
    'oauth_signature_method': 'HMAC-SHA1',
    'oauth_timestamp': timestamp,
    'oauth_token': actual_token,
    'oauth_version': '1.0'
    }
#rq = oauth.OAuthRequest(method, url, parameters)
rq.sign_request(oauth.OAuthSignatureMethod_HMAC_SHA1(),consumer, token);

headers = rq.to_header()
oauth_url = rq.to_url()
print rq.get_normalized_parameters()
print oauth_url
print headers
parts = [x.strip() for x in headers["Authorization"].split(",") if x.find("=") != -1]
smallparts = dict([x.split("=") for x in parts])
print "Signature from Python:", smallparts["oauth_signature"]
