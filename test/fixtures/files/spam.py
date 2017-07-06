#!/usr/bin/env python2
import signal, socket, pickle, zlib, os
import sys
import rsa

entries = {}

def spam_list():
        print("Listing %d passwords:" % len(entries))
        for k, v in entries.iteritems():
                print("%s: %s" % (k,v))
        print("---")

def spam_add():
        print("Enter name of the site: ")
	sys.stdout.flush();
        name = sys.stdin.readline()
        print("Enter password: ")
	sys.stdout.flush();
        pasw = sys.stdin.readline()
        entries[name] = pasw
        print("Password successfully added.")

def spam_del():
        print("Enter name of the site which should be deleted: ")
	sys.stdout.flush();
        name = sys.stdin.readline()
        if name not in entries:
                print("Entry not found.")
        else:
                del entries[name]
                print("Entry successfully deleted.")

def spam_backup():
        print("Your backup: %s\n" % rsa.encrypt(pickle.dumps(entries),pubkey).encode("base64").replace("\n",""))

def spam_restore():
        print("Paste your backup here: ")
	sys.stdout.flush();
        backup = sys.stdin.readline()
        print backup
        global entries
        entries = pickle.loads(rsa.decrypt(backup.decode("base64"),privkey))
        print("Successfully restored %d entries\n" % len(entries))

def show_key():
        print("Here is your key:")
        print(str(pubkey))


print("Welcome to Super Password Authentication Manager (SPAM)!")
sys.stdout.flush();
(pubkey, privkey) = rsa.newkeys(1024)
while 1:
        while 1:
                print("Menu:")
                print("1) List Passwords")
                print("2) Add a Password")
                print("3) Remove a Password")
                print("4) Backup Passwords")
                print("5) Restore backup")
                print("6) Show the key")
		sys.stdout.flush();
		l = str(input())
                if len(l) == 1 and l in "123456":
                        [spam_list, spam_add, spam_del, spam_backup, spam_restore, show_key][int(l) - 1]()
                else:
                        print("Invalid choice.")


