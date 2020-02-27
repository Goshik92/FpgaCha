typedef logic [1:0] CsrAddr_t;

module StreamToMemory(
    input logic clock,
    input logic reset,
    
    // Avalon-MM slave interface for configuring the module
    input logic csr_write,
    input CsrAddr_t csr_address,
    input logic [31:0] csr_writedata,
    
    // Avalon-MM master interface for outputting the data
    output logic m_write,
    output logic [32:0] m_address,
    output logic [255:0] m_writedata,
    output logic [1:0] m_burstcount,
    input logic m_waitrequest,
    
    // Avalon-ST sink interface for consuming the data
    input logic [511:0] snk_data,
    input logic snk_valid,
    output logic snk_ready,
    
    // Interrupt request
    output logic irq
);
    localparam CsrAddr_t LEN_ADDR = 2'h0;
    localparam CsrAddr_t ADDR_ADDR = 2'h1;
    localparam CsrAddr_t IRQ_ADDR = 2'h2;

    logic [31:0] length;
    logic [255:0] buffer;
    logic bufferEmpty;

    // Always use bursts of 2
    assign m_burstcount = 2'h2;
    
    // We are ready to accept data whenever buffer is empty
    assign snk_ready = bufferEmpty;
    
    // Length and address behavior
    always_ff @(posedge clock) begin
        // Reset logic
        if (reset) begin
            length <= 1'b0;
            irq <= 1'b0;
        end
        
        // If len and addr are modified from csr interface
        else if (csr_write) begin
            case(csr_address)
                ADDR_ADDR: m_address <= csr_writedata;
                LEN_ADDR: length <= csr_writedata;
                IRQ_ADDR: irq <= 1'b0;
            endcase
        end
        
        // If data transfer is running
        else if (length != 1'b0) begin
            // Make sure we do not need to stall
            // We stall when:
            //   1. There is no data from the sink or buffer to transfer
            //   2. MM slave asks us to wait
            if ((!bufferEmpty || snk_valid) && !m_waitrequest) begin
               // One more 256-bit word has been transfered
               length <= length - 1'b1;
               
               // Move byte address accordingly
               m_address <= m_address + 32;
               
               //
               if (length == 1'b1) irq <= 1'b1;
            end
        end
    end
    
    // Update dataCounter
    // It keeps track of how many bytes needs
    // to be transfered to Avalon-MM after
    // reading one element from the Avalon-ST
    always_ff @(posedge clock) begin
        // Reset logic
        if (reset) begin
            // No data in the buffer initially
            bufferEmpty <= 1'b1;
            
            // Nothing to transmit to the slave
            m_write <= 1'b0;
        end
        
        // Stall if there is a transfer from csr interface
        else if (csr_write) begin
        end
        
        // Stall if slave cannot accept data
        if (length != 1'b0 && (!m_waitrequest || !m_write)) begin
            // 
            if (bufferEmpty) begin
                // If data from the sink is available
                if (snk_valid) begin
                    // Move data to the buffer
                    m_writedata <= snk_data[511:256];
                    buffer <= snk_data[255:0];
                
                    // Slave can consume first part of data
                    m_write <= 1'b1;
                
                    // 
                    bufferEmpty <= 1'b0;
                end
                
                // If we cannot get valid data,
                // there is nothing to read for slave
                else m_write <= 1'b0;
            end
            
            // If not empty
            else begin
                //
                bufferEmpty <= 1'b1;
                
                //
                m_writedata <= buffer;
                
                // Slave can consume second part of data
                m_write <= 1'b1;
            end
        end
    end
endmodule