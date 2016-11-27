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
import pyautogui
import helpers
from pynput import mouse


COMMUNICATION_KEY_CONFIG = 200
COMMUNICATION_KEY_PING = 100
mouse = mouse.Controller()

CONTEXT_SCROLL = 1
CONTEXT_SOUND = 2
COMMAND_SCROLL = 200
INTENT_CONTEXT_CHANGE = 300
INTENT_MEDIA_PLAY_PAUSE = 500
INTENT_KEY_VOLUME_CHANGE = 501
INTENT_KEY_TRACK_CHANGE = 502
INTENT_PRESENTATION_SLIDE_CHANGE = 600
INTENT_PRESENTATION_FULLSCREEN = 601
COMMAND_TOGGLE = 1
COMMAND_NEXT_UP = 1
COMMAND_PREVIOUS_DOWN = -1
COMMAND_SLIDE_NEXT = 1
COMMAND_SLIDE_PREVIOUS = -1
COMMAND_FULLSCREEN_START = 1
COMMAND_FULLSCREEN_EXIT = 0

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

    return settings

class CommandHandler:
    """ Class for handling the incoming app-messages - commands - from the pebble """
    def __init__(self, settings):
        self.settings = settings

    def message_received_event(self, transaction_id, uuid, data):
        if uuid.get_hex() != self.settings.uuid:
            logging.debug(
                "Ignoring appdata from unknown sender (%s)" %
                data.uuid.get_hex())
            return

        pair_key = data.keys()[0]
        pair_value = data.values()[0]

        if pair_key == INTENT_CONTEXT_CHANGE:
            if pair_value == CONTEXT_SCROLL:
                pass
                # Stop all other stuff, do scroll stuff
            elif pair_value == CONTEXT_SOUND:
                # Stop all other stuff, do media stuff
                pass
            else:
                pass
        elif pair_key == COMMAND_SCROLL:
            mouse_scroll(pair_value)
        elif pair_key == INTENT_MEDIA_PLAY_PAUSE:
            if pair_value == COMMAND_TOGGLE:
                try:
                    helpers.HIDPostAuxKey(helpers.supportedcmds['playpause'])
                except (IndexError):
                    print "\tSupported commands are %s" % helpers.supportedcmds.keys()
            else:
                pass
        elif pair_key == INTENT_KEY_VOLUME_CHANGE:
            if pair_value == COMMAND_NEXT_UP:
                try:
                    helpers.HIDPostAuxKey(helpers.supportedcmds['volup'])
                except (IndexError):
                    print "\tSupported commands are %s" % helpers.supportedcmds.keys()
            elif pair_value == COMMAND_PREVIOUS_DOWN:
                try:
                    helpers.HIDPostAuxKey(helpers.supportedcmds['voldown'])
                except (IndexError):
                    print "\tSupported commands are %s" % helpers.supportedcmds.keys()
            else:
                pass
        elif pair_key == INTENT_KEY_TRACK_CHANGE:
            if pair_value == COMMAND_NEXT_UP:
                try:
                    helpers.HIDPostAuxKey(helpers.supportedcmds['next'])
                except (IndexError):
                    print "\tSupported commands are %s" % helpers.supportedcmds.keys()
            elif pair_value == COMMAND_PREVIOUS_DOWN:
                try:
                    helpers.HIDPostAuxKey(helpers.supportedcmds['prev'])
                except (IndexError):
                    print "\tSupported commands are %s" % helpers.supportedcmds.keys()
            else:
                pass
        elif pair_key == INTENT_PRESENTATION_FULLSCREEN:
            if pair_value == COMMAND_FULLSCREEN_START:
                pyautogui.hotkey('command', 'shift', 'enter')
            elif pair_value == COMMAND_FULLSCREEN_EXIT:
                pyautogui.press('esc')
            else:
                pass
        elif pair_key == INTENT_PRESENTATION_SLIDE_CHANGE:
            if pair_value == COMMAND_SLIDE_NEXT:
                pyautogui.press('right')
            elif pair_value == COMMAND_SLIDE_PREVIOUS:
                pyautogui.press('left')
            else:
                pass
        else: pass

def mouse_scroll(value_scroll):
    threshold = 15
    limiter = 0.015
    if value_scroll < -threshold:
        final_value_scroll = int(limiter * (-value_scroll+threshold))
        mouse.scroll(0, final_value_scroll)

    elif value_scroll > threshold:
        final_value_scroll = int(limiter * (-value_scroll-threshold))
        mouse.scroll(0, final_value_scroll)
    else:
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
            logging.debug(
                "Ignoring appdata from unknown sender (%s)" %
                data.uuid.get_hex())
            return False
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
    pebble.connect()

    if (pebble.connected):
        print("Pebble successfully connected.")

    try:
        pebble.run_async()
    except libpebble2.exceptions.TimeoutError:
        print("Pebble timeouted")
        logging.info("Pebble timeouted")

    # Install app
    AppInstaller(pebble, "../hotpebble.pbw").install()

    # Register service for app messages
    appservice = AppMessageService(pebble)
    handler = CommandHandler(settings)

    commwatch = CommunicationKeeper(settings, appservice)
    appservice.register_handler("nack", commwatch.nack_received)
    appservice.register_handler("ack", commwatch.ack_received)

    # Start the watchapp
    pebble.send_packet(AppRunState(command = 0x01, data=AppRunStateStart(uuid = uuid.UUID(settings.uuid))))

    logging.info("Connection ok, entering to active state...")
    appservice.register_handler("appmessage", handler.message_received_event)

    while True:
        commwatch.send_message({COMMUNICATION_KEY_PING: Uint8(0)})
        time.sleep(10)


if __name__ == "__main__":
    try:
        main(get_settings())
    except PebbleConnectionException as error :
        logging.error("PebbleConnectionException: " + str(error) )
        logging.error("Bailing out!")
