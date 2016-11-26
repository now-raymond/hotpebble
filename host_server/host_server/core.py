#!/usr/bin/env python

import sys
import argparse
import struct
import ConfigParser
import time
import logging
import uuid
from subprocess import call as subprocess_call

from libpebble2.communication import PebbleConnection
from libpebble2.communication.transports.websocket import WebsocketTransport
from libpebble2.communication.transports.serial import SerialTransport
from libpebble2.services.install import AppInstaller
from libpebble2.protocol import *
from libpebble2.communication.transports import *
from libpebble2.services.appmessage import *
from libpebble2.protocol.apps import AppRunStateStart
from libpebble2.protocol.apps import AppRunState
import libpebble2.exceptions

COMMUNICATION_KEY_CONFIG = 200
COMMUNICATION_KEY_PING = 100

class PebbleConnectionException(Exception):
  pass

def get_button_ids():
    for press_key in ["UP", "DOWN", "SELECT"]:
        for press_type in ["SINGLE", "LONG", "MULTI"]:
            yield (press_key, press_type)

def get_settings():
    parser = argparse.ArgumentParser(
        description='Pebble to linux keyboard bridge')
    parser.add_argument("config", help="Set the configuration file")
    settings = parser.parse_args()

    conf = ConfigParser.ConfigParser()
    conf.read(settings.config)

    settings.transport = conf.get('main', 'transport')
    settings.device = conf.get('main', 'device')
    settings.uuid = conf.get('main', 'uuid')

    settings.key_mappings = {}

    for (id_key, id_type) in get_button_ids():
        id_full = id_key + "," + id_type

        try:
            cmd = conf.get('commands', id_full)
        except ConfigParser.NoOptionError:
            continue
        settings.key_mappings[id_key[0] + "," + id_type[0]] = cmd
    print("Settings is good")
    return settings

class CommandHandler:
    """ Class for handling the incoming app-messages - commands - from the pebble """
    def __init__(self, settings):
        self.settings = settings

    def message_received_event(self, transaction_id, uuid, data):
        print("Entering a message event...")
        if uuid.get_hex() != self.settings.uuid:
            print("input uuid and settings uuid don't match")
            logging.debug(
                "Ignoring appdata from unknown sender (%s)" %
                data.uuid.get_hex())
            return
        print("uuids MATCH")
        print(self)
        print(transaction_id)
        print(uuid)
        print(data)
        print(data.get(100))
        print(data.get(101))
        print(data.get(102))
        print(data.get(103))
        test_volume_change(data.get(102))
        # assert (1 in data), "Missing key on data structure"
        # assert (2 in data), "Missing key on data structure"
        #
        # key_id = chr(data[2]) + "," + chr(data[1])
        #
        # if key_id not in self.settings.key_mappings:
        #     logging.warning("Ignoring key press '%s' of unknown id. " % key_id)
        #     return
        # to_emulate = self.settings.key_mappings[key_id]
        # print("Command called: '%s' " % to_emulate)
        # subprocess_call(to_emulate, shell=True)

def test_volume_change(yValue):
    if (yValue > 200):
        # up volume
        pass
    elif (yValue < -200):
        # down volume
        pass
    else:
        # Do not do anything
        pass

class CommunicationKeeper:
    """ Class for handling re-sending of NACK-ed messages """
    NACK_COUNT_LIMIT = 5
    def __init__(self, settings, appservice):
        self.settings = settings
        self.uuid = uuid.UUID(settings.uuid)

        self.pending = {}
        self.nack_count = 0
        self.appservice = appservice
        self.error = None

    def check_uuid(self, uuid):
        if uuid != self.uuid:
            print("unknown sender")
            logging.debug(
                "Ignoring appdata from unknown sender (%s)" %
                data.uuid.get_hex())
            return False
        print("known sender")
        return True

    def nack_received(self, transaction_id, uuid):
        """ Callback functions for the library call when receiving nack """
        if self.check_uuid(uuid) == False:
            return
        if transaction_id not in self.pending:
            raise PebbleConnectionException("Invalid transaction ID received")

        # We got nack from the watch
        logging.warning("NACK received for packet!")
        self.nack_count += 1
        if self.nack_count > self.NACK_COUNT_LIMIT:
            # we are inside the receive thread here, exception will kill only
            # that
            self.error = "Nack count limit reached, something is wrong."
            return
        # self.send_message( self.pending[transaction_id] )
        del self.pending[transaction_id]

    def ack_received(self, transaction_id, uuid):
        logging.debug("ACK received for packet!")
        if self.check_uuid(uuid) == False:
            return
        if transaction_id not in self.pending:
            raise Exception("Invalid transaction ID received")
        del self.pending[transaction_id]

    def send_message(self, data):
        """ Send message and retry sending if it gets nacked """
        transaction_id = self.appservice.send_message(self.uuid, data)
        self.pending[transaction_id] = data

def main(settings):
    """ Main function for the communicator, loops here """

    if settings.transport == "websocket":
        pebble = PebbleConnection(WebsocketTransport(settings.device), log_packet_level=logging.DEBUG)
    else: # No elif, for compatibility with older configs
        pebble = PebbleConnection(SerialTransport(settings.device), log_packet_level=logging.DEBUG)
        print("Connecting to Serial Bluetooth")
    pebble.connect()

    if (pebble.connected):
        print("Pebble successfully connected.")

    # For some reason it seems to timeout for the first time, with "somebody is eating our input" error,
    # replying seems to help.
    try:
        pebble.run_async()
    except libpebble2.exceptions.TimeoutError:
        print("Pebble timeouted")
        logging.info("Pebble timeouted")


    # Register service for app messages
    appservice = AppMessageService(pebble)
    handler = CommandHandler(settings)

    commwatch = CommunicationKeeper(settings, appservice)
    appservice.register_handler("nack", commwatch.nack_received)
    appservice.register_handler("ack", commwatch.ack_received)

    # Start the watchapp
    pebble.send_packet(AppRunState(command = 0x01, data=AppRunStateStart(uuid = uuid.UUID(settings.uuid))))

    # # Send our current config
    # for (id_key, id_type) in get_button_ids():
    #     id_full = id_key[0] + "," + id_type[0]
    #     if id_full in settings.key_mappings:
    #         status = "T"
    #     else:
    #         status = "F"
    #
    #     data = id_key[0] + id_type[0] + status
    #     commwatch.send_message({COMMUNICATION_KEY_CONFIG: CString(data)})
    #
    #     # Wait for all
    #     for loop in range(10):
    #         if len(commwatch.pending) == 0:
    #             break
    #         if commwatch.error:
    #             raise PebbleConnectionException("Commwatch:" + commwatch.error)
    #         time.sleep(0.1)
    #     else:
    #         raise PebbleConnectionException("Pebble not respoding to config")

    print("Connection ok, entering to active state...")
    logging.info("Connection ok, entering to active state...")
    appservice.register_handler("appmessage", handler.message_received_event)

    while True:
        commwatch.send_message({COMMUNICATION_KEY_PING: Uint8(0)})
        time.sleep(10)


if __name__ == "__main__":
    try:
        print("Getting settings now")
        main(get_settings())
    except PebbleConnectionException as error :
        logging.error("PebbleConnectionException: " + str(error) )
        logging.error("Bailing out!")
