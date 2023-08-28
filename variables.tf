variable "compartment_ocid" {
  description = ""
}

variable "instance_name" {
  description = ""
}


variable "instance_shape" {
  description = ""
}

variable "region" {
  description = ""
}

variable "ssh_public_key_path" {
  description = ""
}
variable "ssh_private_key_path" {
  description = ""
}

variable "subnet_cidr" {
  description = ""
}

variable "vcn_cidr" {
  description = ""
}

variable "core_count" {
  description = "core count"
  default     = 1
}

variable "AD_number" {
  type          = number
  description   = ""
  default       = 0
}


variable "DB_PASS" {
  description = "SYSTEM and PDBADMIN password"
  default     = "Tiger$tr0ng2023"
}