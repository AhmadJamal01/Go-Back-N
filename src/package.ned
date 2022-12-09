package data_link_project;

@license(LGPL);
//
// TODO documentation
//
network DataLink
{
    submodules:
        node[2] : Node;
        coordinator : Coordinator;
    connections:
        for i=0..1 {
            coordinator.out++ --> node[i].in++;
            node[i].out --> node[1-i].in++;
        }     
}

