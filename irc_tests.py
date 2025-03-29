import socket
import time
import threading

SERVER_HOST = 'localhost'
SERVER_PORT = 6667
PASSWORD = '123'

class IRCTestClient:
    def __init__(self, nickname):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.nickname = nickname
        self.buffer = ""
        
    def connect(self):
        self.sock.connect((SERVER_HOST, SERVER_PORT))
        self.send(f"PASS {PASSWORD}")
        self.send(f"NICK {self.nickname}")
        self.send(f"USER {self.nickname} 0 * :{self.nickname} User")
        time.sleep(0.1)
        return self.receive()

    def send(self, message):
        print(f"[{self.nickname}] >>> {message}")
        self.sock.sendall(f"{message}\r\n".encode())

    def receive(self):
        data = self.sock.recv(4096).decode(errors='ignore')
        self.buffer += data
        lines = self.buffer.split("\r\n")
        self.buffer = lines.pop()
        for line in lines:
            if line.strip():  # Ignore les lignes vides
                print(f"[{self.nickname}] <<< {line}")
        return lines

def run_test(test_name, callback):
    print(f"\n\033[1;34m=== {test_name.upper()} ===\033[0m")
    try:
        callback()
        print(f"\033[1;32m[✓] {test_name}\033[0m")
    except Exception as e:
        print(f"\033[1;31m[✗] {test_name}: {str(e)}\033[0m")

### Tests spécifiques ###
def test_connection():
    client = IRCTestClient("Tester")
    response = client.connect()
    if not any("001" in line for line in response):
        raise Exception("Connection failed")

def test_nick_collision():
    client1 = IRCTestClient("Duplicate")
    client2 = IRCTestClient("Duplicate")
    client1.connect()
    response = client2.connect()
    if not any("433" in line for line in response):
        raise Exception("Nick collision not detected")

def test_channel_join():
    alice = IRCTestClient("Alice")
    alice.connect()
    alice.send("JOIN #test")
    response = "\n".join(alice.receive())
    if "JOIN" not in response:
        raise Exception("Channel join failed")

def test_private_message():
    alice = IRCTestClient("Alice")
    bob = IRCTestClient("Bob")
    alice.connect()
    bob.connect()
    alice.send("PRIVMSG Bob :Hello private!")
    response = "\n".join(bob.receive())
    if "Hello private!" not in response:
        raise Exception("Private message failed")

def test_channel_message():
    alice = IRCTestClient("Alice")
    bob = IRCTestClient("Bob")
    alice.connect()
    bob.connect()
    alice.send("JOIN #test")
    bob.send("JOIN #test")
    alice.send("PRIVMSG #test :Hello channel!")
    response = "\n".join(bob.receive())
    if "Hello channel!" not in response:
        raise Exception("Channel message failed")

def test_mode_invite_only():
    op = IRCTestClient("Op")
    alice = IRCTestClient("Alice")
    op.connect()
    alice.connect()
    op.send("JOIN #private")
    op.send("MODE #private +i")
    alice.send("JOIN #private")
    response = "\n".join(alice.receive())
    if "473" not in response:
        raise Exception("Invite-only mode failed")

def test_mode_password():
    op = IRCTestClient("Op")
    alice = IRCTestClient("Alice")
    op.connect()
    alice.connect()
    op.send("JOIN #secret")
    op.send("MODE #secret +k password123")
    alice.send("JOIN #secret wrongpass")
    response = "\n".join(alice.receive())
    if "475" not in response:
        raise Exception("Channel password failed")

def test_kick():
    op = IRCTestClient("Op")
    bob = IRCTestClient("Bob")
    op.connect()
    bob.connect()
    op.send("JOIN #moderated")
    bob.send("JOIN #moderated")
    op.send("MODE #moderated +o Op")
    op.send("KICK #moderated Bob :Test")
    response = "\n".join(bob.receive())
    if "KICK" not in response:
        raise Exception("Kick command failed")

def test_invite():
    op = IRCTestClient("Op")
    alice = IRCTestClient("Alice")
    op.connect()
    alice.connect()
    op.send("JOIN #inviteonly")
    op.send("MODE #inviteonly +i")
    op.send("INVITE Alice #inviteonly")
    alice.send("JOIN #inviteonly")
    response = "\n".join(alice.receive())
    if "JOIN" not in response:
        raise Exception("Invite failed")

def test_topic():
    op = IRCTestClient("Op")
    op.connect()
    op.send("JOIN #topic_test")
    op.send("TOPIC #topic_test :New topic")
    op.send("TOPIC #topic_test")
    response = "\n".join(op.receive())
    if "New topic" not in response:
        raise Exception("Topic failed")

def test_part():
    alice = IRCTestClient("Alice")
    alice.connect()
    alice.send("JOIN #part_test")
    alice.send("PART #part_test")
    response = "\n".join(alice.receive())
    if "PART" not in response:
        raise Exception("Part command failed")

def test_quit():
    alice = IRCTestClient("Alice")
    alice.connect()
    alice.send("QUIT :Goodbye")
    response = "\n".join(alice.receive())
    if "ERROR" in response:
        raise Exception("Quit failed")

def main():
    tests = [
        ("Connection", test_connection),
        ("Nick Collision", test_nick_collision),
        ("Channel Join", test_channel_join),
        ("Private Message", test_private_message),
        ("Channel Message", test_channel_message),
        ("Invite-Only Mode", test_mode_invite_only),
        ("Channel Password", test_mode_password),
        ("Kick Command", test_kick),
        ("Invite Command", test_invite),
        ("Topic Command", test_topic),
        ("Part Command", test_part),
        ("Quit Command", test_quit)
    ]

    print("\033[1;36m" + "="*50)
    print("IRC SERVER COMPREHENSIVE TEST SUITE")
    print("="*50 + "\033[0m")

    for name, test in tests:
        run_test(name, test)

    print("\n\033[1;36m=== TESTS COMPLETED ===")
    print(f"Total tests: {len(tests)}")
    print("Check output for any [✗] failures\033[0m")

if __name__ == "__main__":
    main()
