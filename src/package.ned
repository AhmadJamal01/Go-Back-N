package data_link_project;

@license(LGPL);
//
// TODO documentation
//
network DataLink
{
    @display("bgb=694,312");
    submodules:
        node[2]: Node {
            @display("p=329,93;i=device/drive");
        }
        coordinator: Coordinator {
            @display("p=329,221;i=block/cogwheel");
        }
    connections:
        for i=0..1 {
            coordinator.out++ --> node[i].in++;
            node[i].out --> node[1-i].in++;
        }
}

