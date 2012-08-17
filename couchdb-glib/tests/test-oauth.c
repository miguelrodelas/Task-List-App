#include <string.h>
#include <glib.h>
#include <libsoup/soup-uri.h>
#include <oauth.h>
#include <openssl/hmac.h>
#include <couchdb-glib.h>

void
usage (const char *program)
{
	g_print ("Usage: %s consumer_key consumer_secret token_key token_secret url method\n", program);
	exit (-1);
}

char *
openssl_sign (const char *base_signature, const char *key)
{
	unsigned char result[EVP_MAX_MD_SIZE];
	unsigned int resultlen = 0;
  
	HMAC(EVP_sha1(), key, strlen (key), 
	     (unsigned char*) base_signature, strlen (base_signature),
	     result, &resultlen);

	return g_base64_encode ((guchar *) result, resultlen);
}

void
retrieve_nonce_and_timestamp (const char *sign, char **nonce, char **timestamp)
{
	char **parsed_url;

	*nonce = NULL;
	*timestamp = NULL;

	parsed_url = g_strsplit (sign, "?", 2);
	if (parsed_url) {
		gchar **params;
		int i;

		params = g_strsplit ((const gchar *) parsed_url[1], "&", 0);
		for (i = 0; params[i] != NULL; i++) {
			gchar **url_param;

			url_param = g_strsplit ((const gchar *) params[i], "=", 2);
			if (url_param == NULL)
				continue;

			if (g_strcmp0 (url_param[0], "oauth_nonce") == 0)
				*nonce = g_strdup (url_param[1]);
			else if (g_strcmp0 (url_param[0], "oauth_timestamp") == 0)
				*timestamp = g_strdup (url_param[1]);

			g_strfreev (url_param);
		}

		g_strfreev (params);
		g_strfreev (parsed_url);
	}
}

int
main (int argc, char *argv[])
{
	char *base_signature, *oauth_url, *encoded_oauth_url, *oauth_key;
	char *c_key, *c_secret, *t_key, *t_secret, *url, *method, *nonce, *timestamp;
	char *liboauth_signed;
	char *command_line, *command_line_output;
	CouchdbSession *couchdb;
	CouchdbCredentials *credentials;
	GSList *db_list;
	GError *error = NULL;

	if (argc != 7)
		usage (argv[0]);

	c_key = argv[1];
	c_secret = argv[2];
	t_key = argv[3];
	t_secret = argv[4];
	url = argv[5];
	method = argv[6];

	/* First run liboauth signing, which generates nonce and timestamp */
	g_print ("*** Signing with liboauth ***\n");
	liboauth_signed = oauth_sign_url2 (url, NULL, OA_HMAC, method, c_key, c_secret, t_key, t_secret);
	g_print ("oauth_sign_url2 returns: %s\n", liboauth_signed);

	retrieve_nonce_and_timestamp (liboauth_signed, &nonce, &timestamp);

	/* Running openssl signing */
	g_print ("\n*** Signing with openssl ***\n");
	oauth_url = g_strdup_printf (
		"oauth_callback=None&oauth_consumer_key=%s&oauth_nonce=%s&oauth_signature_method=HMAC-SHA1&oauth_timestamp=%s&oauth_token=%s&oauth_verifier=None&oauth_version=1.0",
		c_key, nonce, timestamp, t_key);
	encoded_oauth_url = soup_uri_encode (oauth_url, "=&");
	base_signature = g_strdup_printf ("%s&%s&%s", method, soup_uri_encode (url, NULL), encoded_oauth_url);
	oauth_key = g_strdup_printf ("%s&%s", c_secret, t_secret);

	g_print ("openssl: data to sign='%s'\n", base_signature);
	g_print ("openssl: key='%s'\n", oauth_key);
	g_print ("openssl returns: %s\n", openssl_sign (base_signature, oauth_key));

	/* Running python-oauth script */
	g_print ("\n*** Signing with python-oauth ***\n");
	command_line = g_strdup_printf ("/usr/bin/python test-oauth.py %s %s %s %s %s %s %s %s",
					c_key, c_secret, t_key, t_secret, url, method, nonce, timestamp);
	g_spawn_command_line_sync (command_line, &command_line_output, NULL, NULL, NULL);
	g_print ("%s\n", command_line_output);

	/* Now connecting to Couchdb with this OAuth stuff */
	g_type_init ();
	g_thread_init (NULL);

	couchdb = couchdb_session_new (url);
	credentials = couchdb_credentials_new_with_oauth (c_key, c_secret, t_key, t_secret);
	couchdb_session_enable_authentication (couchdb, credentials);
	g_object_unref (G_OBJECT (credentials));

	db_list = couchdb_session_list_databases (couchdb, &error);
	if (db_list != NULL) {
		GSList *sl;

		for (sl = db_list; sl != NULL; sl = sl->next) {
			g_print ("Found database %s\n", (const char *) sl->data);
		}

		couchdb_session_free_database_list (db_list);
	} else if (error != NULL) {
		g_print ("Could not get list of databases: %s\n", error->message);
		g_error_free (error);
	}

	g_object_unref (G_OBJECT (couchdb));

	return 0;
}
