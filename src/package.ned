package data_link_project;

@license(LGPL);
//
// TODO documentation
//
network DataLink
{
    submodules:
        node0 : Node0;
        node1 : Node1;
        coordinator : Coordinator;
    connections:
        coordinator.out++ --> node0.in++;
        coordinator.out++ --> node1.in++;
        node0.out --> node1.in++;
        node1.out --> node0.in++;
        
        
        
        
        
}

