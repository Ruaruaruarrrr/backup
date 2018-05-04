import java.util.ArrayList;

public class State <E> {

	
	public State (Node<E> currNode, ArrayList<Edge<E>> shipments, State<E> preState,
			int g, int h) {
		
		this.setCurrNode(currNode);
		this.setShipments(shipments);
		this.setPreState(preState);
		this.setG(g);
		this.setH(h);
		this.setF();
		
	}
	
	private Node<E> currNode;
	private ArrayList<Edge<E>> shipments = new ArrayList<Edge<E>>();
	private State<E> preState;
	private int g;
	private int h;
	private int f;
	
	
	/**
	 * calculate heuristic 
	 * @param s
	 * @param weights
	 * @return
	 */
	public int heuristic(int weights) {
		//countShipment()
		//get edge weight from prev node to curr node
		int weight = getPreState().getCurrNode().getEdgeWeight(getCurrNode())
					//+ getPreState().getCurrNode().getCost()
					//+ getCurrNode().getCost()
					;
		
		
		int count = 0;
		//if it also a start of a new shipment
		if (isStart(getCurrNode())) {
			//subtract edge cost twice
		count =  weights-(weight);
		}
		//if reach this node will complete a shipment,
		if (getPreState().check(getCurrNode())) {
			

			
			//Subtract the entire edge cost once
			return weights-(int)(weight/2);
		
		//if it just a start of a new shipment
		} else if (isStart(getCurrNode())) {
			//Subtract the half of the edge cost 
			//return weights-((int)weight/2);
			
		}
		
		return weights;
	}
	
	

	/*public int countShipment() {
		int count = 0;
		for (Edge<E> e : shipments) { 
			//System.out.println(">>>"+
			count += getPreState().getCurrNode().getEdgeWeight(e.getFrom()) 
			+ e.getWeight() +e.getTo().getCost() 
			+e.getFrom().getCost();
		}
		return count; 
	}*/
	
	/**
	 * check if the given node is the start of the shipment or not
	 * 
	 * @param from
	 * @return
	 */
	public boolean isStart(Node<E> from) {
		for (Edge<E> e : shipments) { 
			if (e.getFrom().equals(getCurrNode())) {
				return true;
			}
		}
		return false;
	}
	
	/**
	 * check if there exist a shipment from current Node 
	 * to given node in current state
	 * @param from
	 * @param to
	 * @return true if it exist
	 * 			false if not
	 */
	public boolean check(Node<E> to) {
		for (Edge<E> e : shipments) { 
			if (e.getFrom().equals(getCurrNode()) && e.getTo().equals(to)) {
				return true;
			}
		}
		return false;
		
	}
	
	//count shipments left
	public int count() {

		return shipments.size();
	}
	
	
	
	/**
	 * remove shipment from shipments in current state
	 * @param Nodes
	 * @return time of shipments that removed 
	 */
	public int removeShipments (Node<E> from ,Node<E> to) {
		int sub = 0;
		for (int i=0; i<shipments.size(); i++) { 
			if (shipments.get(i).getFrom().equals(from) &&
				shipments.get(i).getTo().equals(to)) {
				
				sub = shipments.get(i).getWeight() 
					// - shipments.get(i).getFrom().getCost()
					 //- shipments.get(i).getTo().getCost()
						;
				shipments.remove(i);
				break;
				
			}
		}
		return sub;
	}
	
	/**
	 * f() = g() + h()
	 */
	public void setF() {
		this.f = getG() + getH();
	}
	
	
	
	//gets and sets
	public Node<E> getCurrNode() {
		return currNode;
	}
	public void setCurrNode(Node<E> currNode) {
		this.currNode = currNode;
	}
	public ArrayList<Edge<E>> getShipments() {
		return shipments;
	}
	public void setShipments(ArrayList<Edge<E>> shipments) {
		this.shipments = shipments;
	}
	public State<E> getPreState() {
		return preState;
	}
	public void setPreState(State<E> preState) {
		this.preState = preState;
	}
	public int getG() {
		return g;
	}
	public void setG(int g) {
		this.g = g;
	}
	public int getH() {
		return h;
	}
	public void setH(int h) {
		this.h = h;
	}
	public int getF() {
		return f;
	}

	
	//for debugging
	/*public void print() {
		int count = 0;
		for (Edge<E> e : getShipments()) {
			count++;
		}
		System.out.println(count);
	}
	
	*/
}
