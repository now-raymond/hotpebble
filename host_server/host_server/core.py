#!/usr/bin/env python

import sys
import argparse
import struct
import configparser
import time
import logging
import uuid
import os
import threading

from subprocess import call

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
import win32api,win32con

#global variables
COMMUNICATION_KEY_CONFIG = 200
COMMUNICATION_KEY_PING = 100
speed_constant = 0.22
direction = 1
yValue = 0
scroll_thread = None
# Context Keys
context_media = 1
context_scroll = 2
context_presentation = 3

class PebbleConnectionException(Exception):
  pass

def get_button_ids():
    for press_key in ["UP", "DOWN", "SELECT"]:
        for press_type in ["SINGLE", "LONG", "MULTI"]:
            yield (press_key, press_type)


def smooth_scroll_thread():
    while (yValue):
        if yValue > 0:
            direction = 1
        else:
            direction = -1
        win32api.mouse_event(win32con.MOUSEEVENTF_WHEEL, 0, 0, -1 * direction, 0)
        absYValue = abs(yValue)
        time.sleep(speed_constant/absYValue)


def get_settings():
    parser = argparse.ArgumentParser(
        description='Pebble to linux keyboard bridge')
    parser.add_argument("config", help="Set the configuration file")
    settings = parser.parse_args()

    conf = configparser.ConfigParser()
    conf.read(settings.config)

    settings.transport = conf.get('main', 'transport')
    settings.device = conf.get('main', 'device')
    settings.uuid = conf.get('main', 'uuid')

    settings.key_mappings = {}

    for (id_key, id_type) in get_button_ids():
        id_full = id_key + "," + id_type

        try:
            cmd = conf.get('commands', id_full)
        except configparser.NoOptionError:
            continue
        settings.key_mappings[id_key[0] + "," + id_type[0]] = cmd
    print("Settings is good")
    return settings

class CommandHandler:
    """ Class for handling the incoming app-messages - commands - from the pebble """
    def __init__(self, settings):
        self.settings = settings

    def message_received_event(self, transaction_id, uuid, data):
        global scroll_thread,yValue,contextKey,context_scroll,context_media,context_play_pause,context_track_change,context_volume
        global context_presentation
        print("Entering a message event...")
        # if uuid.get_hex() != self.settings.uuid:
        #     print("input uuid and settings uuid don't match")
        #     logging.debug(
        #         "Ignoring appdata from unknown sender (%s)" %
        #         data.uuid.get_hex())
        #     return
        print("uuids MATCH")
        print(self)
        print(transaction_id)
        print(uuid)
        print(data)
        print(data.get(200))
        yValue = data.get(200)
        contextKey = data.get(300)
        context_play_pause = data.get(500)
        context_volume = data.get(501)
        context_track_change = data.get(502)
        context_slide_change = data.get(600)
        context_full_screen = data.get(601)

        if contextKey is not None:
            if contextKey == context_scroll:
                if scroll_thread is None or (not scroll_thread.is_alive() and yValue != 0):
                    scroll_thread = threading.Thread(target=smooth_scroll_thread)
                    scroll_thread.daemon = True
                    scroll_thread.start()
                    # stop media, if any
                    print("Currently in scroll context")
            elif contextKey == context_media:
                yValue = 0 # stop thread
                # TODO: code for media buttons
                # if(context_play_pause == 0 or context_play_pause == 1):
                #     os.system(r'C:\Users\lmy60\Desktop\pauseMedia.ahk')

                # elif(context_volume > 0):
                #     os.system(r'C:\Users\lmy60\Desktop\upVolume.ahk')
                # elif(context_volume < 0):
                #     os.system(r'C:\Users\lmy60\Desktop\downVolume.ahk')
                # elif(context_track_change > 0):
                #     os.system(r'C:\Users\lmy60\Desktop\playNext.ahk')
                # elif(context_track_change < 0):
                #     os.system(r'C:\Users\lmy60\Desktop\playPrev.ahk')
                # else:
                #     pass
                # contextKey = context_media
                print("Entered media context")
            elif contextKey == context_presentation:
                print("Presentation context")
            else:
                pass

        elif context_play_pause is not None:
            print("play/pause!!!")
            os.system('start ./host_server/ahk/pauseMedia.ahk')
            # call(['start','hotpebble/host_server/host_server/ahk/downVolume.ahk'])
        elif context_volume is not None:
            print("In Volume Context!!!")
            if(context_volume > 0):
                print("Up Volume!!!")
                os.system('start ./host_server/ahk/upVolume.ahk')
            else:
                print("Down Volume!!!")
                os.system('start ./host_server/ahk/downVolume.ahk')
        elif yValue is not None:
            if scroll_thread is None or (not scroll_thread.is_alive() and yValue != 0):
                scroll_thread = threading.Thread(target=smooth_scroll_thread)
                scroll_thread.daemon = True
                scroll_thread.start()
                # stop media, if any
                print("Currently in scroll context")
        elif context_track_change is not None:
            print("In Track Context")
            if(context_track_change > 0):
                print("Next Track")
                os.system('start ./host_server/ahk/playNext.ahk')
            else:
                print("Previous Track")
                os.system('start ./host_server/ahk/playPrev.ahk')
        elif context_slide_change is not None:
            print("Slide Change")
            if(context_slide_change > 0):
                print("Next Slide")
                os.system('start ./host_server/ahk/moveRight.ahk')
            else:
                print("previous Slide")
                os.system('start ./host_server/ahk/moveLeft.ahk')
        elif context_full_screen is not None:
            print("full screen context")
            if(context_full_screen > 0):
                print("fullscreen")
                os.system('start ./host_server/ahk/fullScreen.ahk')
            else:
                print("exit full screen")
                os.system('start ./host_server/ahk/exitfullScreen.ahk')

        # if contextKey is not None:
        #     if contextKey == context_media and scroll_thread.is_alive():
        #         # stop thread
        #         yValue = 0
        #         # Do code for media buttons
        #     else:
        #         if scroll_thread is None or (not scroll_thread.is_alive() and yValue != 0):
        #             scroll_thread = threading.Thread(target=smooth_scroll_thread)
        #             scroll_thread.daemon = True
        #             scroll_thread.start()

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
    if (yValue > 100):
        # up volume
        os.system(r'C:\Users\lmy60\Desktop\scrolldown.ahk')
        #subprocess.call(r'C:\Users\lmy60\Desktop\upVolume.ahk')
    elif (yValue < -100):
        # down volume
        os.system(r'C:\Users\lmy60\Desktop\scrollup.ahk')
        #subprocess.call(r'C:\Users\lmy60\Desktop\downVolume.ahk')
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
    #     if uuid != self.uuid:
    #         print("unknown sender")
    #         logging.debug(
    #             "Ignoring appdata from unknown sender (%s)" %
    #             data.uuid.get_hex())
    #         return False
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

    # Install app
    # AppInstaller(pebble, "hotpebble.pbw").install()

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
