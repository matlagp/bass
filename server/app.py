from flask import Flask
from node_repository import Node, NodeRepository
app = Flask(__name__)


@app.route('/')
def hello_world():
    return 'Hello, world!'


if __name__ == '__main__':
    node_repository = NodeRepository()
    node_repository.create_database()
    print(node_repository.create(Node(ip='127.0.0.1')))
    for node in node_repository.all():
        print(node)

    print(node_repository.update(Node(id=5, ip='0.0.0.0')))

    for node in node_repository.all():
        print(node)
    app.run()
