#!/bin/bash
# Tests library functions and types.
#
# Version: 20190216

EXIT_SUCCESS=0;
EXIT_FAILURE=1;
EXIT_IGNORE=77;

LIBRARY_TESTS="access_control_entry analytical_data attribute bit_stream case_data chunk_data chunk_group chunk_table compression data_chunk date_time date_time_values deflate device_information digest_section error error2_section file_entry hash_sections hash_values header_sections header_values huffman_tree io_handle lef_extended_attribute lef_file_entry lef_permission lef_source lef_subject ltree_section md5_hash_section media_values notify permission_group read_io_handle restart_data section_descriptor sector_range segment_file segment_table serialized_string session_section sha1_hash_section single_file_tree single_files source volume_section write_io_handle";
LIBRARY_TESTS_WITH_INPUT="handle support";
OPTION_SETS="";

TEST_TOOL_DIRECTORY=".";
INPUT_DIRECTORY="input";
INPUT_GLOB="*";

run_test()
{
	local TEST_NAME=$1;

	local TEST_DESCRIPTION="Testing: ${TEST_NAME}";
	local TEST_EXECUTABLE="${TEST_TOOL_DIRECTORY}/${TEST_PREFIX}_test_${TEST_NAME}";

	if ! test -x "${TEST_EXECUTABLE}";
	then
		TEST_EXECUTABLE="${TEST_EXECUTABLE}.exe";
	fi

	# TODO: add support for TEST_PROFILE and OPTION_SETS?
	run_test_with_arguments "${TEST_DESCRIPTION}" "${TEST_EXECUTABLE}";
	local RESULT=$?;

	return ${RESULT};
}

run_test_with_input()
{
	local TEST_NAME=$1;

	local TEST_DESCRIPTION="Testing: ${TEST_NAME}";
	local TEST_EXECUTABLE="${TEST_TOOL_DIRECTORY}/${TEST_PREFIX}_test_${TEST_NAME}";

	if ! test -x "${TEST_EXECUTABLE}";
	then
		TEST_EXECUTABLE="${TEST_EXECUTABLE}.exe";
	fi

	run_test_on_input_directory "${TEST_PROFILE}" "${TEST_DESCRIPTION}" "default" "${OPTION_SETS}" "${TEST_EXECUTABLE}" "${INPUT_DIRECTORY}" "${INPUT_GLOB}";
	local RESULT=$?;

	return ${RESULT};
}

if ! test -z ${SKIP_LIBRARY_TESTS};
then
	exit ${EXIT_IGNORE};
fi

TEST_RUNNER="tests/test_runner.sh";

if ! test -f "${TEST_RUNNER}";
then
	TEST_RUNNER="./test_runner.sh";
fi

if ! test -f "${TEST_RUNNER}";
then
	echo "Missing test runner: ${TEST_RUNNER}";

	exit ${EXIT_FAILURE};
fi

source ${TEST_RUNNER};

RESULT=${EXIT_IGNORE};

for TEST_NAME in ${LIBRARY_TESTS};
do
	run_test "${TEST_NAME}";
	RESULT=$?;

	if test ${RESULT} -ne ${EXIT_SUCCESS};
	then
		break;
	fi
done

if test ${RESULT} -ne ${EXIT_SUCCESS} && test ${RESULT} -ne ${EXIT_IGNORE};
then
	exit ${RESULT};
fi

for TEST_NAME in ${LIBRARY_TESTS_WITH_INPUT};
do
	if test -d ${INPUT_DIRECTORY};
	then
		run_test_with_input "${TEST_NAME}";
		RESULT=$?;
	else
		run_test "${TEST_NAME}";
		RESULT=$?;
	fi

	if test ${RESULT} -ne ${EXIT_SUCCESS};
	then
		break;
	fi
done

exit ${RESULT};

