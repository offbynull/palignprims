#include "directed_graph.h"
#include "gtest/gtest.h"

namespace {
    TEST(DirectedGraphTest, InsertNodesAndEdges) {
        directed_graph<std::string, std::string, std::string, std::string> g {};

        g.insert_node("A", "");
        g.insert_node("B", "");
        g.insert_node("C", "");
        g.insert_node("D", "hi!");
        g.insert_node("E", "");
        g.insert_edge("AB1", "A", "B", "MY EDGE DATA FOR 1st AB");
        g.insert_edge("AB2", "A", "B", "MY EDGE DATA FOR 2nd AB");
        g.insert_edge("BC", "B", "C", "");
        g.insert_edge("CD", "C", "D", "");
        g.insert_edge("AD", "A", "D", "");
        g.insert_edge("DE", "D", "E", "");
        {
            auto nodes = g.get_nodes();
            EXPECT_EQ(
                (std::set<std::string> {"A", "B", "C", "D", "E"}),
                (std::set<std::string> (nodes.begin(), nodes.end()))
            );
            auto edges = g.get_edges();
            EXPECT_EQ(
                (std::set<std::string> {"AB1", "AB2", "BC", "CD" ,"AD", "DE"}),
                (std::set<std::string> (edges.begin(), edges.end()))
            );
            EXPECT_TRUE(g.has_node("D"));
            EXPECT_TRUE(g.has_node("E"));
            EXPECT_FALSE(g.has_node("X"));
            EXPECT_EQ(g.get_node_data("A"), "");
            EXPECT_EQ(g.get_node_data("D"), "hi!");
            EXPECT_TRUE(g.has_edge("AD"));
            EXPECT_TRUE(g.has_edge("DE"));
            EXPECT_FALSE(g.has_edge("XE"));
            EXPECT_EQ(g.get_edge_data("AB1"), "MY EDGE DATA FOR 1st AB");
            EXPECT_EQ(g.get_edge_data("BC"), "");
        }
    }

    TEST(DirectedGraphTest, SplitEdge) {
        directed_graph<std::string, std::string, std::string, std::string> g {};

        g.insert_node("A", "");
        g.insert_node("B", "");
        g.insert_node("C", "");
        g.insert_node("D", "hi!");
        g.insert_node("E", "");
        g.insert_edge("AB1", "A", "B", "MY EDGE DATA FOR 1st AB");
        g.insert_edge("AB2", "A", "B", "MY EDGE DATA FOR 2nd AB");
        g.insert_edge("BC", "B", "C", "");
        g.insert_edge("CD", "C", "D", "");
        g.insert_edge("AD", "A", "D", "");
        g.insert_edge("DE", "D", "E", "");
        g.insert_node_between_edge("X", "node data", "DE", "DX", "", "XE", "");
        {
            auto nodes = g.get_nodes();
            EXPECT_EQ(
                (std::set<std::string> {"A", "B", "C", "D", "E", "X"}),
                (std::set<std::string> (nodes.begin(), nodes.end()))
            );
            auto edges = g.get_edges();
            EXPECT_EQ(
                (std::set<std::string> {"AB1", "AB2", "AD", "BC", "CD" ,"DX", "XE"}),
                (std::set<std::string> (edges.begin(), edges.end()))
            );
            EXPECT_FALSE(g.has_edge("DE"));
            EXPECT_TRUE(g.has_edge("DX"));
            EXPECT_TRUE(g.has_edge("XE"));
        }
        {
            EXPECT_EQ(g.get_edge_from("XE"), "X");
            EXPECT_EQ(g.get_edge_to("XE"), "E");
            EXPECT_EQ(g.get_edge_data("XE"), "");
            EXPECT_EQ(std::get<0>(g.get_edge("XE")), "X");
            EXPECT_EQ(std::get<1>(g.get_edge("XE")), "E");
            EXPECT_EQ(std::get<2>(g.get_edge("XE")), "");
        }
    }

    TEST(DirectedGraphTest, DeleteNodesAndEdges) {
        directed_graph<std::string, std::string, std::string, std::string> g {};

        g.insert_node("A", "");
        g.insert_node("B", "");
        g.insert_node("C", "");
        g.insert_node("D", "hi!");
        g.insert_edge("AB1", "A", "B", "MY EDGE DATA FOR 1st AB");
        g.insert_edge("AB2", "A", "B", "MY EDGE DATA FOR 2nd AB");
        g.insert_edge("BC", "B", "C", "");
        g.insert_edge("CD", "C", "D", "");
        g.insert_edge("AD", "A", "D", "");
        g.insert_node("E", "");
        g.insert_edge("DE", "D", "E", "");
        g.insert_node_between_edge("X", "node data", "DE", "DX", "", "XE", "");
        g.delete_edge("AD");
        g.delete_node("X");
        {
            auto nodes = g.get_nodes();
            EXPECT_EQ(
                (std::set<std::string> {"A", "B", "C", "D", "E"}),
                (std::set<std::string> (nodes.begin(), nodes.end()))
            );
            auto edges = g.get_edges();
            EXPECT_EQ(
                (std::set<std::string> {"AB1", "AB2", "BC", "CD"}),
                (std::set<std::string> (edges.begin(), edges.end()))
            );
            EXPECT_FALSE(g.has_node("X"));
            EXPECT_FALSE(g.has_edge("DE"));
            EXPECT_FALSE(g.has_edge("DX"));
            EXPECT_FALSE(g.has_edge("XE"));
        }
    }

    TEST(DirectedGraphTest, UpdateData) {
        directed_graph<std::string, std::string, std::string, std::string> g {};

        g.insert_node("A", "");
        g.insert_node("B", "");
        g.insert_node("C", "");
        g.insert_node("D", "hi!");
        g.insert_edge("AB1", "A", "B", "MY EDGE DATA FOR 1st AB");
        g.insert_edge("AB2", "A", "B", "MY EDGE DATA FOR 2nd AB");
        g.insert_edge("BC", "B", "C", "");
        g.insert_edge("CD", "C", "D", "");
        g.insert_edge("AD", "A", "D", "");

        g.update_node_data("A", "hello world!");
        g.update_edge_data("AB2", "test123");

        {
            EXPECT_EQ(g.get_node_data("A"), "hello world!");
            EXPECT_EQ(g.get_edge_data("AB2"), "test123");
        }

        g.insert_node("E", "");
        g.insert_edge("DE", "D", "E", "");
        g.delete_edge("AD");

        {
            auto nodes = g.get_nodes();
            EXPECT_EQ(
                (std::set<std::string> {"A", "B", "C", "D", "E"}),
                (std::set<std::string> (nodes.begin(), nodes.end()))
            );
            auto edges = g.get_edges();
            EXPECT_EQ(
                (std::set<std::string> {"AB1", "AB2", "BC", "CD" ,"DE"}),
                (std::set<std::string> (edges.begin(), edges.end()))
            );
            EXPECT_FALSE(g.has_edge("AD"));
            EXPECT_TRUE(g.has_edge("DE"));
        }

        g.insert_node_between_edge("X", "node data", "DE", "DX", "", "XE", "");
        {
            auto nodes = g.get_nodes();
            EXPECT_EQ(
                (std::set<std::string> {"A", "B", "C", "D", "E", "X"}),
                (std::set<std::string> (nodes.begin(), nodes.end()))
            );
            auto edges = g.get_edges();
            EXPECT_EQ(
                (std::set<std::string> {"AB1", "AB2", "BC", "CD" ,"DX", "XE"}),
                (std::set<std::string> (edges.begin(), edges.end()))
            );
            EXPECT_FALSE(g.has_edge("DE"));
            EXPECT_TRUE(g.has_edge("DX"));
            EXPECT_TRUE(g.has_edge("XE"));
        }
    }

    TEST(DirectedGraphTest, GetRoots) {
        directed_graph<std::string, std::string, std::string, std::string> g {};

        g.insert_node("A", "");
        g.insert_node("B", "");
        g.insert_node("C", "");
        g.insert_node("D", "hi!");
        g.insert_edge("AB1", "A", "B", "MY EDGE DATA FOR 1st AB");
        g.insert_edge("AB2", "A", "B", "MY EDGE DATA FOR 2nd AB");
        g.insert_edge("BC", "B", "C", "");
        g.insert_edge("CD", "C", "D", "");
        g.insert_edge("AD", "A", "D", "");

        {
            EXPECT_EQ(g.get_root_node(), "A");
            auto nodes = g.get_root_nodes();
            EXPECT_EQ(
                (std::set<std::string> {"A"}),
                (std::set<std::string> (nodes.begin(), nodes.end()))
            );
        }

        g.insert_node("X", "");

        {
            EXPECT_THROW(g.get_root_node(), std::runtime_error);
            auto nodes = g.get_root_nodes();
            EXPECT_EQ(
                (std::set<std::string> {"A", "X"}),
                (std::set<std::string> (nodes.begin(), nodes.end()))
            );
        }
    }

    TEST(DirectedGraphTest, GetLeaves) {
        directed_graph<std::string, std::string, std::string, std::string> g {};

        g.insert_node("A", "");
        g.insert_node("B", "");
        g.insert_node("C", "");
        g.insert_node("D", "hi!");
        g.insert_edge("AB1", "A", "B", "MY EDGE DATA FOR 1st AB");
        g.insert_edge("AB2", "A", "B", "MY EDGE DATA FOR 2nd AB");
        g.insert_edge("BC", "B", "C", "");
        g.insert_edge("CD", "C", "D", "");
        g.insert_edge("AD", "A", "D", "");

        {
            auto nodes = g.get_leaf_nodes();
            EXPECT_EQ(
                (std::set<std::string> {"D"}),
                (std::set<std::string> (nodes.begin(), nodes.end()))
            );
        }

        g.insert_node("X", "");

        {
            auto nodes = g.get_leaf_nodes();
            EXPECT_EQ(
                (std::set<std::string> {"D", "X"}),
                (std::set<std::string> (nodes.begin(), nodes.end()))
            );
        }
    }

    TEST(DirectedGraphTest, HasInputsOutputs) {
        directed_graph<std::string, std::string, std::string, std::string> g {};

        g.insert_node("A", "");
        g.insert_node("B", "");
        g.insert_node("C", "");
        g.insert_node("D", "hi!");
        g.insert_node("E", "");
        g.insert_edge("AB1", "A", "B", "MY EDGE DATA FOR 1st AB");
        g.insert_edge("AB2", "A", "B", "MY EDGE DATA FOR 2nd AB");
        g.insert_edge("BC", "B", "C", "");
        g.insert_edge("CD", "C", "D", "");
        g.insert_edge("AD", "A", "D", "");
        g.insert_edge("DE", "D", "E", "");
        {
            EXPECT_TRUE(g.has_outputs("B"));
            EXPECT_FALSE(g.has_outputs("E"));
        }
        {
            EXPECT_TRUE(g.has_inputs("E"));
            EXPECT_FALSE(g.has_inputs("A"));
        }
    }

    TEST(DirectedGraphTest, GetInputsOutputs) {
        directed_graph<std::string, std::string, std::string, std::string> g {};

        g.insert_node("A", "");
        g.insert_node("B", "");
        g.insert_node("C", "");
        g.insert_node("D", "hi!");
        g.insert_node("E", "");
        g.insert_edge("AB1", "A", "B", "MY EDGE DATA FOR 1st AB");
        g.insert_edge("AB2", "A", "B", "MY EDGE DATA FOR 2nd AB");
        g.insert_edge("BC", "B", "C", "");
        g.insert_edge("CD", "C", "D", "");
        g.insert_edge("AD", "A", "D", "");
        g.insert_edge("DE", "D", "E", "");
        {
            auto edges = g.get_inputs("B");
            EXPECT_EQ(
                (std::set<std::string> {"AB1", "AB2"}),
                (std::set<std::string> (edges.begin(), edges.end()))
            );
        }
        {
            auto edges = g.get_outputs("A");
            EXPECT_EQ(
                (std::set<std::string> {"AB1", "AB2", "AD"}),
                (std::set<std::string> (edges.begin(), edges.end()))
            );
        }
    }

    TEST(DirectedGraphTest, GetFullInputsOutputs) {
        directed_graph<std::string, std::string, std::string, std::string> g {};

        g.insert_node("A", "");
        g.insert_node("B", "");
        g.insert_node("C", "");
        g.insert_node("D", "hi!");
        g.insert_node("E", "");
        g.insert_edge("AB1", "A", "B", "MY EDGE DATA FOR 1st AB");
        g.insert_edge("AB2", "A", "B", "MY EDGE DATA FOR 2nd AB");
        g.insert_edge("BC", "B", "C", "");
        g.insert_edge("CD", "C", "D", "");
        g.insert_edge("AD", "A", "D", "");
        g.insert_edge("DE", "D", "E", "");
        {
            const auto& c_input = g.get_input_full("C");
            EXPECT_EQ(std::get<0>(c_input), "BC");
            EXPECT_EQ(std::get<1>(c_input), "B");
            EXPECT_EQ(std::get<2>(c_input), "C");
            EXPECT_EQ(std::get<3>(c_input), "");
            auto edges = g.get_inputs_full("B");
            auto it = edges.begin();
            EXPECT_EQ(std::get<0>(*it), "AB1");
            EXPECT_EQ(std::get<1>(*it), "A");
            EXPECT_EQ(std::get<2>(*it), "B");
            EXPECT_EQ(std::get<3>(*it), "MY EDGE DATA FOR 1st AB");
            ++it;
            EXPECT_EQ(std::get<0>(*it), "AB2");
            EXPECT_EQ(std::get<1>(*it), "A");
            EXPECT_EQ(std::get<2>(*it), "B");
            EXPECT_EQ(std::get<3>(*it), "MY EDGE DATA FOR 2nd AB");
            ++it;
            EXPECT_EQ(it, edges.end());
        }
        {
            const auto& c_output = g.get_output_full("B");
            EXPECT_EQ(std::get<0>(c_output), "BC");
            EXPECT_EQ(std::get<1>(c_output), "B");
            EXPECT_EQ(std::get<2>(c_output), "C");
            EXPECT_EQ(std::get<3>(c_output), "");
            auto edges = g.get_outputs_full("A");
            auto it = edges.begin();
            EXPECT_EQ(std::get<0>(*it), "AB1");
            EXPECT_EQ(std::get<1>(*it), "A");
            EXPECT_EQ(std::get<2>(*it), "B");
            EXPECT_EQ(std::get<3>(*it), "MY EDGE DATA FOR 1st AB");
            ++it;
            EXPECT_EQ(std::get<0>(*it), "AB2");
            EXPECT_EQ(std::get<1>(*it), "A");
            EXPECT_EQ(std::get<2>(*it), "B");
            EXPECT_EQ(std::get<3>(*it), "MY EDGE DATA FOR 2nd AB");
            ++it;
            EXPECT_EQ(std::get<0>(*it), "AD");
            EXPECT_EQ(std::get<1>(*it), "A");
            EXPECT_EQ(std::get<2>(*it), "D");
            EXPECT_EQ(std::get<3>(*it), "");
            ++it;
            EXPECT_EQ(it, edges.end());
        }
    }

    TEST(DirectedGraphTest, InputOutputDegree) {
        directed_graph<std::string, std::string, std::string, std::string> g {};

        g.insert_node("A", "");
        g.insert_node("B", "");
        g.insert_node("C", "");
        g.insert_node("D", "hi!");
        g.insert_node("E", "");
        g.insert_edge("AB1", "A", "B", "MY EDGE DATA FOR 1st AB");
        g.insert_edge("AB2", "A", "B", "MY EDGE DATA FOR 2nd AB");
        g.insert_edge("BC", "B", "C", "");
        g.insert_edge("CD", "C", "D", "");
        g.insert_edge("AD", "A", "D", "");
        g.insert_edge("DE", "D", "E", "");
        {
            EXPECT_EQ(g.get_out_degree("D"), 1u);
            EXPECT_EQ(g.get_out_degree("E"), 0u);
        }
        {
            EXPECT_EQ(g.get_in_degree("B"), 2u);
            EXPECT_EQ(g.get_in_degree("A"), 0u);
        }
    }

}