from flask import Flask, request, jsonify
from flask_sqlalchemy import SQLAlchemy
from werkzeug.security import generate_password_hash, check_password_hash
import sqlite3

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///players.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db = SQLAlchemy(app)

# Player model
class Player(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    email = db.Column(db.String(100), unique=True, nullable=False)
    password_hash = db.Column(db.String(128), nullable=False)
    personal_info = db.Column(db.String(500))
    pos_x = db.Column(db.Integer, default=0)
    pos_y = db.Column(db.Integer, default=0)

    def set_password(self, password):
        self.password_hash = generate_password_hash(password)

    def check_password(self, password):
        return check_password_hash(self.password_hash, password)

# Initialize database
@app.before_first_request
def create_tables():
    db.create_all()

# Register new player
@app.route('/register', methods=['POST'])
def register():
    data = request.get_json()
    new_player = Player(email=data['email'], personal_info=data.get('personal_info', ''))
    new_player.set_password(data['password'])
    db.session.add(new_player)
    db.session.commit()
    return jsonify({'message': 'Player registered successfully!'}), 201

# Login player
@app.route('/login', methods=['POST'])
def login():
    data = request.get_json()
    player = Player.query.filter_by(email=data['email']).first()
    if player and player.check_password(data['password']):
        return jsonify({'message': 'Login successful!', 'player_id': player.id}), 200
    return jsonify({'message': 'Invalid credentials!'}), 401

# Update player position
@app.route('/update_position', methods=['POST'])
def update_position():
    data = request.get_json()
    player = Player.query.get(data['player_id'])
    if player:
        player.pos_x = min(max(0, data['pos_x']), 149)
        player.pos_y = min(max(0, data['pos_y']), 149)
        db.session.commit()
        return jsonify({'message': 'Position updated successfully!'}), 200
    return jsonify({'message': 'Player not found!'}), 404

# Get player position
@app.route('/get_position/<int:player_id>', methods=['GET'])
def get_position(player_id):
    player = Player.query.get(player_id)
    if player:
        return jsonify({'pos_x': player.pos_x, 'pos_y': player.pos_y}), 200
    return jsonify({'message': 'Player not found!'}), 404

# SCRUM-9 Minimalist game server done

if __name__ == '__main__':
    app.run(debug=True)
