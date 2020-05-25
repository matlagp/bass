from flask import Flask
from repositories import NodeRepository
from models import Node

app = Flask(__name__)


@app.route('/nodes')
def nodes_index():
    node_repository = NodeRepository()
    return '<br>'.join([str(node) for node in node_repository.all()])

@app.route('/nodes/<node_id>')
def nodes_show(node_id=0):
    node_repository = NodeRepository()
    return str(node_repository.find(node_id))


if __name__ == '__main__':
    node_repository = NodeRepository()
    node_repository.create_database()
    # print(node_repository.create(Node(ip='127.0.0.1')))
    # for node in node_repository.all():
    #     print(node)

    # print(node_repository.update(Node(id=5, ip='0.0.0.0')))

    # for node in node_repository.all():
    #     print(node)

    # node_repository.delete(10)
    # node_repository.delete(10)
    app.run()
