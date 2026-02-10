#flask creates server; request lets us read what browser/pico sends
#jsonify turns python data into JSON to send back
#render_temp.. lets us send a simple HTML page without a separate file
from flask import Flask, request, jsonify, render_template_string
#defaultdict is a dictionary that auto-creates entries
#deque is a simple queue like a list (good for adding/removing messages)
from collections import defaultdict, deque
#time gives current time
import time

#creates the server
app = Flask(__name__)

#message storage: eventually each device will have it's own, see below
# inboxes[device_id] = queue of messages
# Merged: using a list for simplicity but keeping the defaultdict logic
INBOXES = defaultdict(list)

#hardcoded pot_group for testing
pot_group = {
    "lilbud_01": "TEST",
    "lilbud_02": "TEST",
    "lilbud_03": "TEST"
}

groups = {
    "TEST": {"lilbud_01", "lilbud_02", "lilbud_03"}
}

#HTML for the website
HOME = """
<h2>Lil Bud Broker</h2>
<form method="post" action="/send">
  <label>From:</label><input name="from_id" value="lilbud_01"><br>
  <label>To:</label><input name="to_id" value="lilbud_02"><br>
  <label>Message:</label><input name="msg" value="🌱 hi"><br><br>
  <button type="submit">Send</button>
</form>
<p>Check inbox: <code>/inbox?device_id=lilbud_02</code></p>
"""

#opens website
@app.get("/")
def home():
    return render_template_string(HOME)

@app.route("/join", methods=["GET", "POST"])
def join():
    if request.method == "POST":
        pass
    return "Join Page Placeholder"

#sends a message
@app.post("/send")
def send():
    #accepts messages from anywhere: website, pico, future devices
    data = request.get_json(silent=True) or request.form.to_dict()
    
    #who sent it
    from_id = data.get("from_id", "unknown")
    #who it's for
    to_id = data.get("to_id", "") #optional
    #message content (supports 'msg' or 'message' keys)
    msg = data.get("msg") or data.get("message")

    #safety check
    if not to_id or not msg:
        return jsonify({"ok": False, "error": "to_id and msg required"}), 400

    #takes message and puts it into the receiver's mailbox
    msg_obj = {
        "from_id": from_id, # matched to your app.py key
        "message": msg,     # matched to your app.py key
        "ts": int(time.time())
    }
    
    INBOXES[to_id].append(msg_obj)
    
    #checking
    print(f"SEND {from_id} → {to_id}: {msg}")
    print("INBOX COUNTS:", {k: len(v) for k, v in INBOXES.items()})
    
    #yes message was delivered
    return jsonify({"ok": True}), 200

#how pots receive messages
@app.get("/inbox")
def inbox():
    device_id = request.args.get("device_id", "")
    #safety check
    if not device_id:
        return jsonify({"ok": False, "error": "device_id required"}), 400
    
    #check mail, then empty mailbox
    msgs = INBOXES.get(device_id, [])
    INBOXES[device_id] = [] 
    
    #sends messages back as JSON
    return jsonify({"ok": True, "messages": msgs})

#start the server
if __name__ == "__main__":
    # host="0.0.0.0" makes it accessible on the local network
    app.run(host="0.0.0.0", port=5000, debug=True, use_reloader=False)
