#flask creates server; request lets us read what browser/pico sends
#jsonify turns python data into JSON to send back
#render_temp.. lets us send a simple HTML page without a separate file
from flask import Flask, request, jsonify, render_template_string
#defaultdict is a dictionary that auto-creates entries
#deque is a simple queue like a list (good for adding/removing messages)
from collections import defaultdict, deque
#time gives current time
import time

#hardcoded pot_group for testing
pot_group = {
    "lilbud_01": "TEST",
    "lilbud_02": "TEST",
    "lilbud_03": "TEST"
    }

groups = {
    "TEST": {"lilbud_01", "lilbud_02", "lilbud_03"}
    }

#creates the server
app = Flask(__name__)

#message storage: eventually each device will have it's own, see below
# inboxes[device_id] = queue of messages
inboxes = defaultdict(lambda: deque(maxlen=50))

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

#sends a message
@app.post("/send")
def send():
    #accepts messages from anywhere: website, pico, future devices
    data = request.get_json(silent=True) or request.form.to_dict()
    #who sent it
    from_id = data.get("from_id", "unknown")
    #who it's for
    to_id = data.get("to_id", "") #optional
    #message content
    msg = data.get("msg", "")
    #safety check
    if not to_id or not msg:
        return jsonify({"ok": False, "error": "to_id and msg required"}), 400
    
    # #directly send to one pot if specific id provided (implement later)
    # if to_id:
    #     inboxes[to_id].append({
    #         "from": from_id, "to": to_id, "msg": msg, "ts": int(time.time())
    #     })
    #     return jsonify({"ok": True, "mode": "direct", "delivered_to": [to_id]})
    
    #takes message and puts it into the receiver's mailbox
    inboxes[to_id].append({
        "from": from_id,
        "to": to_id,
        "msg": msg,
        "ts": int(time.time())
    })
    print(f"SEND {from_id} → {to_id}: {msg}")
    #yes message was delivered
    return jsonify({"ok": True})

#how pots receive messages
@app.get("/inbox")
def inbox():
    device_id = request.args.get("device_id", "")
    #safety check
    if not device_id:
        return jsonify({"ok": False, "error": "device_id required"}), 400
    
    #check mail, then empty mailbox
    msgs = list(inboxes[device_id])
    inboxes[device_id].clear()
    #sends messages back as JSON
    return jsonify({"ok": True, "messages": msgs})

#start the server
if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True, use_reloader=False)
