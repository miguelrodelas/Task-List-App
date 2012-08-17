#!/usr/bin/python
# Copyright 2008 Canonical Ltd.  All rights reserved.
# sil, Feb 2009

import os, random, string, sys
from couchdb import Server
from uuid import uuid4

CONTACT_DOCTYPE = "http://example.com/contact-record"

first_names = [ "Jack", "Thomas", "Oliver", "Joshua", "Harry", "Charlie",
    "Daniel", "William", "James", "Alfie", "Grace", "Ruby", "Olivia",
    "Emily", "Jessica", "Sophie", "Chloe", "Lily", "Ella", "Amelia" ]
last_names = [ "Dalglish", "Grobbelaar", "Lawrenson", "Beglin", "Nicol",
    "Whelan", "Hansen", "Johnston", "Rush", "Molby", "MacDonald", "McMahon" ]
cities = ["Scunthorpe", "Birmingham", "Cambridge", "Durham", "Bedford"]
countries = ["England", "Ireland", "Scotland", "Wales"]
companies = ["ACME Ltd", "Trotter Trading", "Utopia Enterprises"]
titles = ["Engineer", "Manager", "Director", "Vice President", "CEO"]

def randomString(length=10):
    "Return a string of random lowercase letters and of specified length"
    return ''.join([random.choice(string.lowercase) for x in range(length)])

def randomPhoneNumber():
    "Return something that looks like a phone number"
    return "+%s %s %s %s" % (random.randint(10, 99), random.randint(10, 99),
      random.randint(1000, 9999), random.randint(1000, 9999))

def createContacts(local_port, database_name, number_of_contacts):
    server_url = 'http://localhost:%s/' % local_port
    server = Server(server_url)
    if database_name in server:
        db = server[database_name]
    else:
        db = server.create(database_name)
    
    for maincount in range(1, number_of_contacts + 1):
        # Record schema fields
        fielddict = {
            "record_type": CONTACT_DOCTYPE,
            "record_type_version": "1.0"
        }
        
        # Name fields
        first_name = random.choice(first_names) + str(maincount)
        last_name = random.choice(last_names)
        
        # Addresses
        addresses = {}
        for addresscount in range(2):
            description = ["home", "work", "other"][addresscount]
            address = {
              "address1": "%s Street" % random.choice(last_names),
              "address2": "",
              "pobox": "",
              "city": random.choice(cities),
              "state": "",
              "postalcode": "%s12 3%s" % (randomString(2), randomString(2)),
              "country": random.choice(countries),
              "description": description
            }
            uuid = str(uuid4())
            addresses[uuid] = address
            
        # Email addresses
        email_addresses = {}
        for eaddresscount in range(2):
            eaddress = {
              "address": "%s.%s@%s.com" % (
                first_name, last_name, randomString(3)
              ),
              "description": ["home", "work", "other"][eaddresscount]
            }
            uuid = str(uuid4())
            email_addresses[uuid] = eaddress
        
        # Phone numbers
        phone_numbers = {}
        for pncount in range(3):
            pn = {
              "priority": 0,
              "number": randomPhoneNumber(),
              "description": ["home", "work", "other"][pncount]
            }
            uuid = str(uuid4())
            phone_numbers[uuid] = pn
            if pn["description"] == "home": saved_home_uuid = uuid
        
        # Store data in fielddict
        fielddict.update({
            "first_name": first_name,
            "last_name": last_name,
            "addresses": addresses,
            "email_addresses": email_addresses,
            "phone_numbers": phone_numbers,
            "birth_date": "%04d-%02d-%02d" % (random.randint(1900, 2006),
                random.randint(1, 12), random.randint(1, 28))
        })
        
        # Add example application annotations
        fielddict["application_annotations"] = {
            "Funambol": {
                "application_fields": {
                    "jobTitle": random.choice(titles),
                    "company": random.choice(companies)
                },
                "private_application_annotations": {
                    "phoneHome": saved_home_uuid
                }
            }
        }
        
        # Store data in CouchDB
        db.create(fielddict)

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] in ("-h", "--help"):
        print "Parameters: local_port database_name number_of_contacts_to_create"
        sys.exit()
    local_port = int(sys.argv[1]) if len(sys.argv) > 1 else 5984
    database_name = sys.argv[2] if len(sys.argv) > 2 else "contacts"
    number_of_contacts = int(sys.argv[3]) if len(sys.argv) > 3 else 10
    createContacts(local_port, database_name, number_of_contacts)
    print "Created %s contacts at http://localhost:%s/_utils/database.html?%s" % (
      number_of_contacts, local_port, database_name)

