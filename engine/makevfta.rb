#
#       part of this code is origined from
#       GOGO-no-coda
#
#	Copyright (C) 2002, 2003 gogo-developer
#
# TABSIZE = 2

require 'singleton';

module Makevfta
	FileHeader = <<FILE_HEADER
/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 *
 *   [!] This is auto generated file by Makevfta.rb for gogo-no-coda. [!]
 */
FILE_HEADER
end;

class VtaH 
	include Singleton
	include Makevfta
	def initialize
		@vftas = Array::new;
	end;

	def add(vfta_name, return_type, arg, cpu_ids)
		@vftas << [vfta_name, cpu_ids, return_type, arg];
	end;
	
	def write_file
		vfta_functions = '';
		first = true;
		Supported_cpus.each do | cpu_id, dummy |
			if first then 
				vfta_functions += "#if   defined(#{cpu_id})\n";
			else
				vfta_functions += "#elif defined(#{cpu_id})\n";
			end;
			@vftas.each do | vfta_contents |
				vfta_name = vfta_contents[0]
				if vfta_contents[1].include?(cpu_id) then
					vfta_functions += 
						"\tEXT #{vfta_contents[2]} (*#{vfta_name})(#{vfta_contents[3]});\n";
				else
					vfta_functions += 
						"\textern #{vfta_contents[2]} #{vfta_name}_C(#{vfta_contents[3]});\n";
					vfta_functions += 
						"\t#define #{vfta_name} #{vfta_name}_C\n";
				end;
			end;
			first = false;
		end;
		vfta_functions += "#endif\n";
		File::open('vfta.h', 'w') do |f|
			f.write(<<VTA_H
#{FileHeader}
#ifndef GOGO_VFTA_H
#define GOGO_VFTA_H

#include "quantize_pvt.h"

#{vfta_functions}
#endif // GOGO_VFTA_H
VTA_H
			);
		end;
	end;
end;

class SetupC
	include Singleton
	include Makevfta
	def initialize
		@cpus   = Hash::new;
	end;

	def add(setup_name, cpu_ids)
		cpu_ids.each do | cpu_id |
			Supported_cpus.raise_unless_cpu_id_supported(cpu_id);
			if not @cpus[cpu_id] then
				@cpus[cpu_id] = Array::new;
			end;
			@cpus[cpu_id] << setup_name;
		end;
	end;
	
	def write_file
		setup_function_defs  = '';
		setup_function_calls = '';
		first = true;
		@cpus.each do | cpu_id, setup_names |
			if first then 
				setup_function_defs  += "#if   defined(#{cpu_id})\n";
				setup_function_calls += "#if   defined(#{cpu_id})\n";
			else
				setup_function_defs  += "#elif defined(#{cpu_id})\n";
				setup_function_calls += "#elif defined(#{cpu_id})\n";
			end;
			setup_names.each do | setup_name |
				setup_function_defs  += "\textern void #{setup_name}( int unit );\n";
				setup_function_calls += "\t#{setup_name}( unit );\n";
			end;
			first = false;
		end;
		setup_function_defs  += "#endif\n";
		setup_function_calls += "#endif\n";
		File::open('setup.c', 'w') do |f|
			f.write(<<SETUP_C
#{FileHeader}
#include \"cpu.h\"

#{setup_function_defs}
void
setupUNIT(int unit)
{
#{setup_function_calls}} /* setupUNIT */
SETUP_C
			);
		end;
	end;
end;

module Makevfta
	Supported_cpus = Hash::new;

	def Supported_cpus.raise_unless_cpu_id_supported(id)
		if not has_key?(id) then
			raise "Unknown CPU_ID:#{id}";
		end;
	end;
end;

def cpu(cpu_id, cpu_dir)
	Makevfta::Supported_cpus[cpu_id] = cpu_dir;
end;

def setup(setup_name, cpu_ids)
	Makevfta::SetupC.instance.add(setup_name, cpu_ids)
end;

def vfta(vfta_name, return_type, arg, cpu_ids)
	Makevfta::VtaH.instance.add(vfta_name, return_type, arg, cpu_ids)
end;

eval( File::readlines( "vfta.txt" ).join.untaint );
Makevfta::SetupC.instance.write_file;
Makevfta::VtaH.instance.write_file;
