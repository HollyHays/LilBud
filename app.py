from flask import Flask, render_template, request

app = Flask(__name__)

@app.route("/join", methods=["GET", "POST"])
def join():
    if request.method == "POST":
        # handle join logic here
        pass
    return render_template("join.html")

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True, use_reloader=False)
