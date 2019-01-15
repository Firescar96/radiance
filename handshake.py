from socket import socket, AF_INET, SOCK_DGRAM
from dtls import do_patch
import requests
import binascii
import sslpsk
import json

BRIDGE_IP = '192.168.1.198'
VALID_API_VERSION = 1.22
USERNAME = 'QFD1UAkcgF5AfVWslmmm2KdsphGhvyrXa5gxjWKr'
CLIENT_KEY = '65810C9DFAF47E77BF39E918DBF9B4FA'
STREAMING_PORT = 2100


# Check current version of Hue bridge, needs to be > 1.22 for Entertainment stuff
def check_bridge_version(ip):
    url = 'http://%s/api/config' % ip
    r = requests.get(url).json()
    api_version_raw = r['apiversion']
    final_period = api_version_raw.rfind('.')
    api_version = float(api_version_raw[0:final_period])

    if api_version >= VALID_API_VERSION:
        return True

    return False


def get_entertainment_groups(ip):
    url = 'http://%s/api/%s/groups' % (ip, USERNAME)
    r = requests.get(url)
    groups = r.json()
    entertainment_groups = {}

    for group_id in groups:
        group = groups[group_id]
        if group['type'] == 'Entertainment':
            entertainment_groups[group_id] = group

    return entertainment_groups


def toggle_group_streaming(group_id, state):
    url = 'http://%s/api/%s/groups/%s' % (BRIDGE_IP, USERNAME, group_id)
    data = {
        'stream': {
            'active': state
        }
    }
    body = json.dumps(data)
    r = requests.put(url, data=body, timeout=5)
    print r.json()


def dtls_handshake():
    print("start handshake")
    psk = bin(int(binascii.hexlify(CLIENT_KEY), 16))
    do_patch()
    print("after patch")
    s = socket(AF_INET, SOCK_DGRAM)
    s.connect((BRIDGE_IP, STREAMING_PORT))
    print("connected")
    sock = sslpsk.wrap_socket(s,
                              psk=(psk,USERNAME))
    message = 'ping'
    print "sending message"
    sock.sendto(message, (BRIDGE_IP, STREAMING_PORT))
    print "sent"
    msg = s.recv(4)
    print 'Client received: %s' % msg


def entertainment_registration(ip):
    url = 'http://%s/api/' % ip
    data = {
        'devicetype': 'ScreenBloom',
        'generateclientkey': True
    }
    body = json.dumps(data)
    r = requests.post(url, data=body, timeout=5)

if __name__ == '__main__':
    if check_bridge_version(BRIDGE_IP):
        print 'Bridge API version is OK!'
        # entertainment_registration(BRIDGE_IP)

        GROUPS = get_entertainment_groups(BRIDGE_IP)
        toggle_group_streaming(2, True)
        dtls_handshake()
        toggle_group_streaming(2, False)
    else:
        print 'Bridge API version is not recent enough for Hue Entertainment.'
        print 'Make sure you\'re using a 2.0 or greater Bridge (square) and are running the latest firmware!'
