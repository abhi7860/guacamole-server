
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is libguac.
 *
 * The Initial Developer of the Original Code is
 * Michael Jumper.
 * Portions created by the Initial Developer are Copyright (C) 2010
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */


#ifndef _CLIENT_H
#define _CLIENT_H

#include <png.h>

#include "guacio.h"
#include "protocol.h"

/**
 * Provides functions and structures required for defining (and handling) a proxy client.
 *
 * @file client.h
 */

/**
 * The time to allow between sync responses in milliseconds. If a sync
 * instruction is sent to the client and no response is received within this
 * timeframe, server messages will not be handled until a sync instruction is
 * received from the client.
 */
#define GUAC_SYNC_THRESHOLD 500

/**
 * The time to allow between server sync messages in milliseconds. A sync
 * message from the server will be sent every GUAC_SYNC_FREQUENCY milliseconds.
 * As this will induce a response from a client that is not malfunctioning,
 * this is used to detect when a client has died. This must be set to a
 * reasonable value to avoid clients being disconnected unnecessarily due
 * to timeout.
 */
#define GUAC_SYNC_FREQUENCY 5000

/**
 * The amount of time to wait after handling server messages. If a client
 * plugin has a message handler, and sends instructions when server messages
 * are being handled, there will be a pause of this many milliseconds before
 * the next call to the message handler.
 */
#define GUAC_SERVER_MESSAGE_HANDLE_FREQUENCY 50

typedef struct guac_client guac_client;

/**
 * Handler for server messages (where "server" refers to the server that
 * the proxy client is connected to).
 */
typedef int guac_client_handle_messages(guac_client* client);

/**
 * Handler for Guacamole mouse events.
 */
typedef int guac_client_mouse_handler(guac_client* client, int x, int y, int button_mask);

/**
 * Handler for Guacamole key events.
 */
typedef int guac_client_key_handler(guac_client* client, int keysym, int pressed);

/**
 * Handler for Guacamole clipboard events.
 */
typedef int guac_client_clipboard_handler(guac_client* client, char* copied);

/**
 * Handler for freeing up any extra data allocated by the client
 * implementation.
 */
typedef int guac_client_free_handler(void* client);

typedef enum guac_client_state {
    RUNNING,
    STOPPING
} guac_client_state;

/**
 * Guacamole proxy client.
 *
 * Represents a Guacamole proxy client (the client which communicates to
 * a server on behalf of Guacamole, on behalf of the web-client).
 */
struct guac_client {

    /**
     * The GUACIO structure to be used to communicate with the web-client. It is
     * expected that the implementor of any Guacamole proxy client will provide
     * their own mechanism of I/O for their protocol. The GUACIO structure is
     * used only to communicate conveniently with the Guacamole web-client.
     */
    GUACIO* io;

    guac_client_state state;
    long last_received_timestamp;
    long last_sent_timestamp;

    /**
     * Reference to dlopen'd client plugin.
     */
    void* client_plugin_handle;

    /**
     * Arbitrary reference to proxy client-specific data. Implementors of a
     * Guacamole proxy client can store any data they want here, which can then
     * be retrieved as necessary in the message handlers.
     */
    void* data;

    /**
     * Handler for server messages. If set, this function will be called
     * occasionally by the Guacamole proxy to give the client a chance to
     * handle messages from whichever server it is connected to.
     *
     * Example:
     * @code
     *     void handle_messages(guac_client* client);
     *
     *     int guac_client_init(guac_client* client, int argc, char** argv) {
     *         client->handle_messages = handle_messages;
     *     }
     * @endcode
     */
    guac_client_handle_messages* handle_messages;

    /**
     * Handler for mouse events sent by the Gaucamole web-client.
     *
     * The handler takes the integer mouse X and Y coordinates, as well as
     * a button mask containing the bitwise OR of all button values currently
     * being pressed. Those values are:
     *
     * <table>
     *     <tr><th>Button</th>          <th>Value</th></tr>
     *     <tr><td>Left</td>            <td>1</td></tr>
     *     <tr><td>Middle</td>          <td>2</td></tr>
     *     <tr><td>Right</td>           <td>4</td></tr>
     *     <tr><td>Scrollwheel Up</td>  <td>8</td></tr>
     *     <tr><td>Scrollwheel Down</td><td>16</td></tr>
     * </table>

     * Example:
     * @code
     *     void mouse_handler(guac_client* client, int x, int y, int button_mask);
     *
     *     int guac_client_init(guac_client* client, int argc, char** argv) {
     *         client->mouse_handler = mouse_handler;
     *     }
     * @endcode
     */
    guac_client_mouse_handler* mouse_handler;

    /**
     * Handler for key events sent by the Guacamole web-client.
     *
     * The handler takes the integer X11 keysym associated with the key
     * being pressed/released, and an integer representing whether the key
     * is being pressed (1) or released (0).
     *
     * Example:
     * @code
     *     void key_handler(guac_client* client, int keysym, int pressed);
     *
     *     int guac_client_init(guac_client* client, int argc, char** argv) {
     *         client->key_handler = key_handler;
     *     }
     * @endcode
     */
    guac_client_key_handler* key_handler;

    /**
     * Handler for clipboard events sent by the Guacamole web-client. This
     * handler will be called whenever the web-client sets the data of the
     * clipboard.
     *
     * This handler takes a single string which contains the text which
     * has been set in the clipboard. This text is already unescaped from
     * the Guacamole escaped version sent within the clipboard message
     * in the protocol.
     *
     * Example:
     * @code
     *     void clipboard_handler(guac_client* client, char* copied);
     *
     *     int guac_client_init(guac_client* client, int argc, char** argv) {
     *         client->clipboard_handler = clipboard_handler;
     *     }
     * @endcode
     */
    guac_client_clipboard_handler* clipboard_handler;

    /**
     * Handler for freeing data when the client is being unloaded.
     *
     * This handler will be called when the client needs to be unloaded
     * by the proxy, and any data allocated by the proxy client should be
     * freed.
     *
     * Implement this handler if you store data inside the client.
     *
     * Example:
     * @code
     *     void free_handler(guac_client* client);
     *
     *     int guac_client_init(guac_client* client, int argc, char** argv) {
     *         client->free_handler = free_handler;
     *     }
     * @endcode
     */
    guac_client_free_handler* free_handler;

};

/**
 * Handler which should initialize the given guac_client.
 */
typedef int guac_client_init_handler(guac_client* client, int argc, char** argv);

/**
 * Initialize and return a new guac_client. The pluggable client will be chosen based on
 * the first connect message received on the given file descriptor.
 *
 * @param client_fd The file descriptor associated with the socket associated with the connection to the
 *                  web-client tunnel.
 * @return A pointer to the newly initialized client.
 */
guac_client* guac_get_client(int client_fd);

/**
 * Enter the main network message handling loop for the given client.
 *
 * @param client The proxy client to start handling messages of/for.
 */
void guac_start_client(guac_client* client);

/**
 * Free all resources associated with the given client.
 *
 * @param client The proxy client to free all reasources of.
 */
void guac_free_client(guac_client* client);

/**
 * Allocate a libpng-compatible buffer to hold raw image data.
 *
 * @param w The width of the buffer to allocate, in pixels.
 * @param h The height of the buffer to allocate, in pixels.
 * @param bpp The number of bytes per pixel (3 for RGB images, 4 for RGBA).
 * @return A pointer to the newly allocated buffer.
 */
png_byte** guac_alloc_png_buffer(int w, int h, int bpp);

/**
 * Free all memory associated with the given libpng-compatible buffer
 * as allocated by guac_alloc_png_buffer.
 *
 * @param png_buffer The buffer to free.
 * @param h The height of the buffer to free.
 */
void guac_free_png_buffer(png_byte** png_buffer, int h);

int guac_client_handle_instruction(guac_client* client, guac_instruction* instruction);
void guac_client_stop(guac_client* client);

/* FIXME: MOVE THESE TO protocol.h */
long guac_client_current_timestamp();
void guac_client_sleep(int millis);

#endif
