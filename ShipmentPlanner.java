import java.io.FileNotFoundException;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.LinkedList;
import java.util.PriorityQueue;
import java.util.Scanner;

public class ShipmentPlanner {
	
	public static void main(String[] args) {
		//store all cites
		GraphImp<String> AllNodes = new GraphImp<String>();
		//store the required shipments
		ArrayList<Edge<String>> shipments = new ArrayList<Edge<String>>();
		//means the best solution it just pass all the shipments
		int minPath=0;
		
		
		Scanner sc = null;
		Scanner ls = null;
		try {
			sc = new Scanner(new FileReader(args[0]));
			while (sc.hasNextLine()) {
				String line = sc.nextLine();
				ls = new Scanner(line);
				while (ls.hasNext()) {
					String keyword = ls.next();		
					
					if (keyword.equals("Refuelling")) {
						int RefuelTime = ls.nextInt();
						String city = ls.next();
						
						//add node to map
						AllNodes.addNode(city, RefuelTime);
						
					} else if (keyword.equals("Time")) {
						int cost = ls.nextInt();
						String start = ls.next();
						String end = ls.next();
						Node<String> startNode = AllNodes.getNode(start);
						Node<String> endNode = AllNodes.getNode(end);
						
						//add edge to map in both directions
						AllNodes.addEdge(startNode, endNode, cost);
						AllNodes.addEdge(endNode, startNode, cost);
						
					} else if (keyword.equals("Shipment")) {
						String start = ls.next();
						String end = ls.next();
						Node<String> Start = AllNodes.getNode(start);
						Node<String> End = AllNodes.getNode(end);
						
						//add one shipment to the list
						shipments.add(new Edge<String>(Start,End,Start.getEdgeWeight(End)));
						//add weight of edges in shipments to minimum path cost for heuristic
						int Weight = AllNodes.getWeight(Start, End);
						//minPath += Weight;
						minPath += Weight 
								//+ Start.getCost() + End.getCost()
								;
					} else if (keyword.equals("#")) {
						break;
					}
				}
			}
			
			//always start with Sydney
			Node<String> start = AllNodes.getNode("Sydney");
			//call A*
			State<String> thePath = aStarSearch(start,shipments,minPath);
			//print path
			printPath(thePath);
			
		} catch (FileNotFoundException e) {}
		finally {
			if (sc != null)
				sc.close();
			if (ls != null)
				ls.close();
		}
	}
	


	/**
	 * A* Search
	 * @param from
	 * @param shipments
	 * @param minPathTime
	 * @return
	 */

	private static <E> State<E> aStarSearch(Node<E> from, ArrayList<Edge<E>> shipments, int minPathTime) {
		
		//Initialize a comparator for priorityQueue 
		Comparator<State<E>> comp = new StateComparator<E>();
		//openSet will store states that we still have to explore
		PriorityQueue<State<E>> openSet = new PriorityQueue<State<E>>(comp);
		
		//closeSet will store states that we have already evaluated
		ArrayList<State<E>> closeSet = new ArrayList<State<E>>();
		
		//Initialize the first state, add it to the openSet
		State<E> firstState = new State<E>(from, shipments, null, 0, minPathTime);
		openSet.add(firstState);
		System.out.println(minPathTime);
		//count number of nodes expanded
		int nodesExpanded = 0;

		while (!openSet.isEmpty()) {
			
			//count expanded nodes
			nodesExpanded++;
			
			//remove the state with the smallest f in PriorityQueue
			State<E> currState = openSet.poll();
			Node<E> currNode = currState.getCurrNode();
			System.out.println(currState.getCurrNode().getName() + " : " + currState.getH());
			//add this state to the closeSet
			closeSet.add(currState);
			
			// check if all required shipments have finished
			if (currState.count() == 0) {
				System.out.println(nodesExpanded + " nodes expanded");
				//int cost  = currState.getG() + currState.countShipment();
				//System.out.println("cost = " + cost);
				System.out.println("cost = " + currState.getG());
				return currState;
			}
			
			//we choose neighbors(Nodes) that are in current shipments expect itself
			ArrayList<Node<E>> Neighbours = new ArrayList<Node<E>>();
			for (Edge<E> e : currState.getShipments()) {
				//find from node and add it
				if (!e.getFrom().equals(currNode) && !Neighbours.contains(e.getFrom())) { 
					Neighbours.add(e.getFrom());
				}
				//find to node and add it
				if (!e.getTo().equals(currNode) && !Neighbours.contains(e.getTo())) {
					Neighbours.add(e.getTo());
				}
			}
			
			for (Node<E> n : Neighbours) {
				//get g value
				int g = currState.getG() + currNode.getEdgeWeight(n) + currNode.getCost();
				
				//create new state for this neighbor
				ArrayList<Edge<E>> NewShipments = new ArrayList<Edge<E>>(currState.getShipments());
				State<E> NewState = new State<E>(n, NewShipments, currState, g, 0);
				
				int sub = 0;
				//check if it is the existed shipment
				if (currState.check(n)) {
					//to complete the shipment then remove it
					sub = NewState.removeShipments(currNode,n);
				}

				//calculate the heuristic
				int h = NewState.heuristic (currState.getH() - sub );
				//int h = NewState.heuristic (minPathTime-sub);
				//store h value
				NewState.setH(h);
				//calculate f value
				NewState.setF();
				//System.out.println(NewState.getF());

				//add it to the priorityQueue
				openSet.add(NewState);	
			}
		}
		return null;
	}
	
	
	

	
	/**
	 * print path
	 * @param curr
	 */
	private static <E> void printPath(State<E> curr) {
		//store the path into an array 
		LinkedList<State<E>> path = new LinkedList<State<E>>();
		while (curr != null) {
			path.addFirst(curr);
			curr = curr.getPreState();
		}
		
		//print the list
		for (int i = 0; i < path.size()-1; i++) {
			System.out.println("Ship " + path.get(i).getCurrNode().getName()
					+ " to " + path.get(i+1).getCurrNode().getName());
		}
	}
	
	
	
	
	
	
}