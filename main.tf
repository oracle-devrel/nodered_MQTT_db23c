# Create datasource of images from the image list
data "oci_core_images" "images" {
  compartment_id = var.compartment_ocid
  operating_system = "Oracle Linux"
  operating_system_version = "8"
  shape = var.instance_shape
}


# Create a compute instance with a public IP address using oci provider
resource "oci_core_instance" "instance" {
  availability_domain = data.oci_identity_availability_domains.ADs.availability_domains[var.AD_number].name
  compartment_id      = var.compartment_ocid
  display_name        = var.instance_name
  shape               = var.instance_shape


  
  source_details {
    source_type = "image"
    source_id   = data.oci_core_images.images.images[var.AD_number].id
    boot_volume_size_in_gbs = 50
  }

  create_vnic_details {
    assign_public_ip = "true"
    subnet_id        = oci_core_subnet.subnet.id
  }


  # Add private key
  metadata = {
    ssh_authorized_keys = file(var.ssh_public_key_path)
    user_data           = base64encode(file("install.sh"))
  }
  shape_config {
        #Optional
        # memory_in_gbs = "16"
        memory_in_gbs = var.core_count*2 <16 ? 16 : var.core_count*2
        ocpus = var.core_count
  }


  connection {
    type        = "ssh"
    host        = "${self.public_ip}"
    user        = "opc"
    private_key = "${file(var.ssh_private_key_path)}"
  }

  provisioner "remote-exec" {
    on_failure = continue
    inline = [
      "wget -qO - 'https://raw.githubusercontent.com/badr42/nodered_MQTT_db23c/main/database.sh' | sudo bash ",
      "wget -qO - 'https://raw.githubusercontent.com/badr42/nodered_MQTT_db23c/main/config_db.sh' | bash ",
      "wget -qO - 'https://raw.githubusercontent.com/badr42/nodered_MQTT_db23c/main/install.sh' | bash ",  
    ]
  }
}

# Create datasource for availability domains
data "oci_identity_availability_domains" "ADs" {
  compartment_id = var.compartment_ocid 
}

# Create internet gateway
resource "oci_core_internet_gateway" "internet_gateway" {
  compartment_id = var.compartment_ocid
  vcn_id         = oci_core_virtual_network.dbfree_vcn.id
  display_name   = "dbfree-internet-gateway"
}

# Create route table
resource "oci_core_route_table" "dbfree_route_table" {
  compartment_id = var.compartment_ocid
  vcn_id         = oci_core_virtual_network.dbfree_vcn.id
  display_name   = "dbfree-route-table"
  route_rules {
    destination = "0.0.0.0/0"
    network_entity_id = oci_core_internet_gateway.internet_gateway.id
  }
}

# Create security list with ingress and egress rules
resource "oci_core_security_list" "dbfree_security_list" {
  compartment_id = var.compartment_ocid
  vcn_id         = oci_core_virtual_network.dbfree_vcn.id
  display_name   = "dbfree-security-list"

  egress_security_rules {
    destination = "0.0.0.0/0"
    protocol    = "all"
    description = "Allow all outbound traffic"
  }

  ingress_security_rules {
    protocol    = "all"
    source      = "0.0.0.0/0"
    description = "Allow all inbound traffic"
  }

  # ingress rule for ssh
    ingress_security_rules {
        protocol    = "6" # tcp
        source      = "0.0.0.0/0"
        description = "Allow ssh"
        tcp_options {
            max = 22
            min = 22
        }
    }
}

# Create a subnet
resource "oci_core_subnet" "subnet" {
  cidr_block        = var.subnet_cidr
  compartment_id    = var.compartment_ocid
  display_name      = "dbfree-subnet"
  vcn_id            = oci_core_virtual_network.dbfree_vcn.id
  route_table_id    = oci_core_route_table.dbfree_route_table.id
  security_list_ids = ["${oci_core_security_list.dbfree_security_list.id}"]
  dhcp_options_id   = oci_core_virtual_network.dbfree_vcn.default_dhcp_options_id
}

# Create a virtual network
resource "oci_core_virtual_network" "dbfree_vcn" {
  cidr_block     = var.vcn_cidr
  compartment_id = var.compartment_ocid
  display_name   = "dbfree-vcn"
}

output "instance_public_ip" {
  value = <<EOF

  to connect to node-red server 
  http://${oci_core_instance.instance.public_ip}:1880
  
  to connect to the database 

  ssh -i server.key opc@${oci_core_instance.instance.public_ip}

  sudo su - oracle 

  #set the environment variables 
  export ORACLE_SID=FREE 
  export ORAENV_ASK=NO 
  . /opt/oracle/product/23c/dbhomeFree/bin/oraenv
  cd $ORACLE_HOME/bin
  lsnrctl status
  ./sqlplus / as sysdba


EOF
}
