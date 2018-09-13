/*******************************************************************************
 * Copyright (c) 2009 David Findlay
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    - Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
#include "AdvancedRemote.h"
#include "util/printf.h"


//#define DEBUG

AdvancedRemote::CallbackHandler defhandler;

AdvancedRemote::AdvancedRemote() {
    handler = &defhandler;
    currentlyEnabled=false;
}

void AdvancedRemote::setCallbackHandler(CallbackHandler &h) {
    handler = &h;
}

void AdvancedRemote::getRemoteMode() {
#ifdef DEBUG
	dmsg("iPOD getRemoteMode()");
#endif
    sendCommand(MODE_SWITCHING_MODE,0x03);
}

void AdvancedRemote::enable()
{
#ifdef DEBUG
	dmsg("iPOD enable()");
#endif
    sendCommand(MODE_SWITCHING_MODE, 0x01, ADVANCED_REMOTE_MODE);
}

void AdvancedRemote::disable()
{
#ifdef DEBUG
	dmsg("iPOD disable()");
#endif
    sendCommand(MODE_SWITCHING_MODE, 0x01, SIMPLE_REMOTE_MODE);
}

void AdvancedRemote::getiPodName()
{
    sendCommand(ADVANCED_REMOTE_MODE, 0x00, CMD_GET_IPOD_NAME);
}

void AdvancedRemote::switchToMainLibraryPlaylist()
{
    sendCommand(ADVANCED_REMOTE_MODE, 0x00, CMD_SWITCH_TO_MAIN_LIBRARY_PLAYLIST);
}

void AdvancedRemote::switchToItem(AdvancedRemote::ItemType itemType, long index)
{
    sendCommandWithOneByteAndOneNumberParam(ADVANCED_REMOTE_MODE, 0x00, CMD_SWITCH_TO_ITEM, itemType, index);
}

void AdvancedRemote::getItemCount(AdvancedRemote::ItemType itemType)
{
    sendCommandWithOneByteParam(ADVANCED_REMOTE_MODE, 0x00, CMD_GET_ITEM_COUNT, itemType);
}

void AdvancedRemote::getItemNames(AdvancedRemote::ItemType itemType, unsigned long offset, unsigned long count)
{
     sendCommandWithOneByteAndTwoNumberParams(ADVANCED_REMOTE_MODE, 0x00, CMD_GET_ITEM_NAMES, itemType, offset, count);
}

void AdvancedRemote::getTimeAndStatusInfo()
{
    sendCommand(ADVANCED_REMOTE_MODE, 0x00, CMD_GET_TIME_AND_STATUS_INFO);
}

void AdvancedRemote::getPlaylistPosition()
{
    sendCommand(ADVANCED_REMOTE_MODE, 0x00, CMD_GET_PLAYLIST_POSITION);
}

void AdvancedRemote::getTitle(unsigned long index)
{
#ifdef DEBUG
	dprintf("getTitle(%d)\n\r",(int)index);
#endif
    sendCommandWithOneNumberParam(ADVANCED_REMOTE_MODE, 0x00, CMD_GET_TITLE, index);
}

void AdvancedRemote::getArtist(unsigned long index)
{
    sendCommandWithOneNumberParam(ADVANCED_REMOTE_MODE, 0x00, CMD_GET_ARTIST, index);
}

void AdvancedRemote::getAlbum(unsigned long index)
{
    sendCommandWithOneNumberParam(ADVANCED_REMOTE_MODE, 0x00, CMD_GET_ALBUM, index);
}

void AdvancedRemote::setPollingMode(AdvancedRemote::PollingMode newMode)
{
    sendCommandWithOneByteParam(ADVANCED_REMOTE_MODE, 0x00, CMD_POLLING_MODE, newMode);
}

void AdvancedRemote::controlPlayback(AdvancedRemote::PlaybackControl command)
{
    sendCommandWithOneByteParam(ADVANCED_REMOTE_MODE, 0x00, CMD_PLAYBACK_CONTROL, command);
}

void AdvancedRemote::getShuffleMode()
{
    sendCommand(ADVANCED_REMOTE_MODE, 0x00, CMD_GET_SHUFFLE_MODE);
}

void AdvancedRemote::setShuffleMode(AdvancedRemote::ShuffleMode newMode)
{
    sendCommandWithOneByteParam(ADVANCED_REMOTE_MODE, 0x00, CMD_SET_SHUFFLE_MODE, newMode);
}

void AdvancedRemote::getRepeatMode()
{
    sendCommand(ADVANCED_REMOTE_MODE, 0x00, CMD_GET_REPEAT_MODE);
}

void AdvancedRemote::setRepeatMode(AdvancedRemote::RepeatMode newMode)
{
    sendCommandWithOneByteParam(ADVANCED_REMOTE_MODE, 0x00, CMD_SET_REPEAT_MODE, newMode);
}

void AdvancedRemote::getSongCountInCurrentPlaylist()
{
    sendCommand(ADVANCED_REMOTE_MODE, 0x00, CMD_GET_SONG_COUNT_IN_CURRENT_PLAYLIST);
}

void AdvancedRemote::executeSwitchAndJumpToSong(unsigned long index)
{
    sendCommandWithOneNumberParam(ADVANCED_REMOTE_MODE, 0x00, CMD_EXECUTE_SWITCH_AND_JUMP_TO_SONG, index);
}

void AdvancedRemote::jumpToSongInCurrentPlaylist(unsigned long index)
{
    sendCommandWithOneNumberParam(ADVANCED_REMOTE_MODE, 0x00, CMD_JUMP_TO_SONG_IN_CURRENT_PLAYLIST, index);
}

void AdvancedRemote::processData()
{
    const byte mode = dataBuffer[0];

    if(mode == MODE_SWITCHING_MODE && dataBuffer[1]==0x02 && dataBuffer[2]==0x04) {
    	currentlyEnabled = false;
    	handler->handleRemoteMode(false);
#ifdef DEBUG
    	dmsg("simple remote mode");
#endif
    	return;
    }

    if(mode == MODE_SWITCHING_MODE && dataBuffer[1]==0x04 && dataBuffer[2]==0x01) {
    	currentlyEnabled = true;
    	handler->handleRemoteMode(true);
#ifdef DEBUG
    	dmsg("advanced remote mode");
#endif
    	return;
    }

    if (mode != ADVANCED_REMOTE_MODE)
    {
#ifdef DEBUG
        dmsg("response not for adv mode so ignoring");
        dumpReceive();
#endif
        return;
    }

    const byte firstCommandByte = dataBuffer[1];
    if (firstCommandByte != 0x00)
    {
#ifdef DEBUG
        dmsg("1st cmd byte in response not 0x00 so ignoring");
        dumpReceive();
#endif
         return;
    }

    const byte secondCommandByte = dataBuffer[2];
    switch (secondCommandByte)
    {
    case RESPONSE_BAD:

#ifdef DEBUG
		dprintf("BAD Response: Result=0x%02x, command=0x%02x, 0x%02x"dataBuffer[3],dataBuffer[4],dataBuffer[5]);
#endif
        return;

    case RESPONSE_FEEDBACK:
        /*
         * Result:
         * 0=success,
         * 2=failure,
         * 4=you exceeded the limit of whatever you were requesting/wrong parameter-count,
         * 5=sent an iPod Response instead of a command
         */
#ifdef DEBUG
		dprintf("Feedback Response: Result=0x%02x, command=0x%02x, 0x%02x"dataBuffer[3],dataBuffer[4],dataBuffer[5]);
#endif

        const Feedback feedback = (Feedback) dataBuffer[3];
        handler->handleFeedback(feedback, dataBuffer[5]);
        return;
    }
    // if we made it past that, hopefully this is a response
    // to a command that we sent

    const byte commandThisIsAResponseFor = secondCommandByte - 1;
    // -1 because response number is always cmd number + 1

    const byte *pData = &dataBuffer[3];
    switch (commandThisIsAResponseFor)
    {
    case CMD_GET_IPOD_NAME:
    	handler->handleIPodName((const char *) pData);
        break;

    case CMD_GET_ITEM_COUNT:
        handler->handleItemCount(endianConvert(pData));
        break;

    case CMD_GET_ITEM_NAMES:
	{
        const unsigned long itemOffset = endianConvert(pData);
        const char *itemName = (const char *) (pData + 4);
        handler->handleItemName(itemOffset, itemName);
	}
        break;

    case CMD_GET_TIME_AND_STATUS_INFO:
    {
        const unsigned long trackLength = endianConvert(pData);
        const unsigned long elapsedTime = endianConvert(pData + 4);
        PlaybackStatus playbackStatus = (PlaybackStatus) *(pData + 8);

        handler->handleTimeAndStatus(trackLength, elapsedTime, playbackStatus);
    }
        break;

    case CMD_GET_PLAYLIST_POSITION:
        handler->handlePlaylistPosition(endianConvert(pData));
        break;

    case CMD_GET_TITLE:
        handler->handleTitle((const char *) pData);
        break;

    case CMD_GET_ARTIST:
        handler->handleArtist((const char *) pData);
        break;

    case CMD_GET_ALBUM:
        handler->handleAlbum((const char *) pData);
        break;

    case CMD_POLLING_MODE:
        handler->handlePolling(endianConvert(pData));
        break;

    case CMD_GET_SHUFFLE_MODE:
        handler->handleShuffleMode((ShuffleMode) *pData);
        break;

    case CMD_GET_REPEAT_MODE:
        handler->handleRepeatMode((RepeatMode) *pData);
        break;

    case CMD_GET_SONG_COUNT_IN_CURRENT_PLAYLIST:
        handler->handleCurrentPlaylistSongCount(endianConvert(pData));
        break;

    default:
#ifdef DEBUG
        dmsg("unsupported response: ");
        dumpReceive();
#endif
        break;
    }
}

/*
 * iPod is big endian and arduino is little endian,
 * so we must byte swap the iPod's 4-byte integers
 * before we can use them on the arduino.
 * note that on arduino int is 16-bit and long is 32-bit.
 * TODO: Is that true of all the micrcontrollers on
 * arduino boards?
 */
unsigned long AdvancedRemote::endianConvert(const byte *p)
{
    return
        (((unsigned long) p[3]) << 0)  |
        (((unsigned long) p[2]) << 8)  |
        (((unsigned long) p[1]) << 16) |
        (((unsigned long) p[0]) << 24);
}

bool AdvancedRemote::isCurrentlyEnabled()
{
    return currentlyEnabled;
}
